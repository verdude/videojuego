"""Parse Aseprite sprite-sheet JSON and play its frame timings.

The module deliberately keeps parsing and playback independent from pygame.  A
game can use :attr:`AnimationFrame.atlas_rect` with ``Surface.subsurface`` and
use :attr:`AnimationFrame.source_position` when an exported frame was trimmed.
"""

from __future__ import annotations

from collections.abc import Mapping
from dataclasses import dataclass
import json
from pathlib import Path
from typing import Any, Self


class AsepriteFormatError(ValueError):
    """Raised when an Aseprite JSON document is missing or has invalid data."""


@dataclass(frozen=True, slots=True)
class Rect:
    """An integer rectangle in a sprite sheet."""

    x: int
    y: int
    width: int
    height: int

    @property
    def tuple(self) -> tuple[int, int, int, int]:
        """Return ``(x, y, width, height)``, suitable for ``pygame.Rect``."""

        return self.x, self.y, self.width, self.height


@dataclass(frozen=True, slots=True)
class Size:
    width: int
    height: int

    @property
    def tuple(self) -> tuple[int, int]:
        return self.width, self.height


@dataclass(frozen=True, slots=True)
class AnimationFrame:
    """One frame and its placement in the exported sprite sheet."""

    name: str
    atlas_rect: Rect
    duration_ms: int
    rotated: bool
    trimmed: bool
    source_rect: Rect
    source_size: Size

    @property
    def source_position(self) -> tuple[int, int]:
        """Where the trimmed image belongs in the untrimmed source frame."""

        return self.source_rect.x, self.source_rect.y


@dataclass(frozen=True, slots=True)
class AnimationTag:
    """A named frame range exported from Aseprite."""

    name: str
    first_frame: int
    last_frame: int
    direction: str = "forward"
    repeat: int | None = None

    def frame_indices(self) -> tuple[int, ...]:
        forward = tuple(range(self.first_frame, self.last_frame + 1))
        if self.direction == "forward":
            return forward
        if self.direction == "reverse":
            return tuple(reversed(forward))
        if self.direction == "pingpong":
            return forward + tuple(reversed(forward[1:-1]))
        if self.direction == "pingpong_reverse":
            reverse = tuple(reversed(forward))
            return reverse + forward[1:-1]
        raise AsepriteFormatError(
            f"tag {self.name!r} has unsupported direction {self.direction!r}"
        )


class Animation:
    """Parsed sprite-sheet metadata with a small, stateful frame player.

    Call :meth:`update` once per game tick with elapsed milliseconds, then read
    :attr:`current_frame`.  ``for_tag`` creates an independent player for a
    named Aseprite frame tag.
    """

    def __init__(
        self,
        frames: tuple[AnimationFrame, ...],
        *,
        image: str,
        sheet_size: Size,
        scale: float = 1.0,
        tags: Mapping[str, AnimationTag] | None = None,
        loop: bool = True,
        _frame_order: tuple[int, ...] | None = None,
    ) -> None:
        if not frames:
            raise AsepriteFormatError("animation must contain at least one frame")

        self.frames = frames
        self.image = image
        self.sheet_size = sheet_size
        self.scale = scale
        self.tags = dict(tags or {})
        self.loop = loop
        self.playing = True
        self._frame_order = _frame_order or tuple(range(len(frames)))
        if not self._frame_order:
            raise AsepriteFormatError("animation frame order cannot be empty")
        if any(index < 0 or index >= len(frames) for index in self._frame_order):
            raise AsepriteFormatError("animation frame order is out of range")
        self._position = 0
        self._elapsed_ms = 0

    @property
    def current_frame(self) -> AnimationFrame:
        return self.frames[self._frame_order[self._position]]

    @property
    def current_frame_index(self) -> int:
        """The current frame's index in the original Aseprite export."""

        return self._frame_order[self._position]

    @property
    def elapsed_ms(self) -> int:
        """Time already spent displaying the current frame."""

        return self._elapsed_ms

    @property
    def duration_ms(self) -> int:
        """Duration of one full playback cycle."""

        return sum(self.frames[index].duration_ms for index in self._frame_order)

    def update(self, delta_ms: int) -> AnimationFrame:
        """Advance playback by ``delta_ms`` and return the resulting frame."""

        if isinstance(delta_ms, bool) or not isinstance(delta_ms, int):
            raise TypeError("delta_ms must be an integer")
        if delta_ms < 0:
            raise ValueError("delta_ms cannot be negative")
        if not self.playing or delta_ms == 0:
            return self.current_frame

        # A complete loop returns to exactly the same frame and elapsed time.
        # Reducing very large clock jumps avoids needlessly walking millions of
        # frames after, for example, resuming a suspended game.
        if self.loop:
            delta_ms %= self.duration_ms
        self._elapsed_ms += delta_ms
        while self._elapsed_ms >= self.current_frame.duration_ms:
            self._elapsed_ms -= self.current_frame.duration_ms
            if self._position + 1 < len(self._frame_order):
                self._position += 1
            elif self.loop:
                self._position = 0
            else:
                self._position = len(self._frame_order) - 1
                self._elapsed_ms = 0
                self.playing = False
                break
        return self.current_frame

    def reset(self) -> AnimationFrame:
        """Rewind to the first playback frame and resume playing."""

        self._position = 0
        self._elapsed_ms = 0
        self.playing = True
        return self.current_frame

    def for_tag(self, name: str, *, loop: bool | None = None) -> Self:
        """Return a new player restricted to the named Aseprite frame tag."""

        try:
            tag = self.tags[name]
        except KeyError as error:
            raise KeyError(f"unknown animation tag {name!r}") from error
        return type(self)(
            self.frames,
            image=self.image,
            sheet_size=self.sheet_size,
            scale=self.scale,
            tags=self.tags,
            loop=self.loop if loop is None else loop,
            _frame_order=tag.frame_indices(),
        )


def parse_aseprite(data: Mapping[str, Any]) -> Animation:
    """Parse a decoded Aseprite JSON mapping into an :class:`Animation`."""

    if not isinstance(data, Mapping):
        raise AsepriteFormatError("the JSON root must be an object")

    raw_frames = _mapping(data, "frames", "root")
    frames = tuple(
        _parse_frame(name, value, index)
        for index, (name, value) in enumerate(raw_frames.items())
    )

    meta = _mapping(data, "meta", "root")
    image = _string(meta, "image", "meta")
    sheet_size = _size(_mapping(meta, "size", "meta"), "meta.size")
    scale = _scale(meta.get("scale", 1))
    tags = _parse_tags(meta.get("frameTags", []), len(frames))

    return Animation(
        frames,
        image=image,
        sheet_size=sheet_size,
        scale=scale,
        tags=tags,
    )


def loads_aseprite(source: str | bytes | bytearray) -> Animation:
    """Parse an Aseprite JSON string or byte string."""

    try:
        data = json.loads(source)
    except (json.JSONDecodeError, UnicodeDecodeError) as error:
        raise AsepriteFormatError(f"invalid JSON: {error}") from error
    return parse_aseprite(data)


def load_aseprite(path: str | Path) -> Animation:
    """Load an Aseprite JSON file from ``path``."""

    json_path = Path(path)
    try:
        source = json_path.read_text(encoding="utf-8")
    except OSError as error:
        raise OSError(f"could not read Aseprite JSON {json_path}: {error}") from error
    return loads_aseprite(source)


def _parse_frame(name: Any, value: Any, index: int) -> AnimationFrame:
    context = f"frames[{index}]"
    if not isinstance(name, str) or not name:
        raise AsepriteFormatError(f"{context} must have a non-empty string name")
    if not isinstance(value, Mapping):
        raise AsepriteFormatError(f"{context} must be an object")

    atlas_rect = _rect(_mapping(value, "frame", context), f"{context}.frame")
    source_rect = _rect(
        _mapping(value, "spriteSourceSize", context),
        f"{context}.spriteSourceSize",
    )
    source_size = _size(
        _mapping(value, "sourceSize", context), f"{context}.sourceSize"
    )
    duration = _integer(value, "duration", context, minimum=1)
    rotated = _boolean(value, "rotated", context)
    trimmed = _boolean(value, "trimmed", context)

    if source_rect.x + source_rect.width > source_size.width:
        raise AsepriteFormatError(f"{context}.spriteSourceSize exceeds source width")
    if source_rect.y + source_rect.height > source_size.height:
        raise AsepriteFormatError(f"{context}.spriteSourceSize exceeds source height")

    return AnimationFrame(
        name=name,
        atlas_rect=atlas_rect,
        duration_ms=duration,
        rotated=rotated,
        trimmed=trimmed,
        source_rect=source_rect,
        source_size=source_size,
    )


def _parse_tags(raw_tags: Any, frame_count: int) -> dict[str, AnimationTag]:
    if not isinstance(raw_tags, list):
        raise AsepriteFormatError("meta.frameTags must be an array")
    tags: dict[str, AnimationTag] = {}
    for index, raw_tag in enumerate(raw_tags):
        context = f"meta.frameTags[{index}]"
        if not isinstance(raw_tag, Mapping):
            raise AsepriteFormatError(f"{context} must be an object")
        name = _string(raw_tag, "name", context)
        first = _integer(raw_tag, "from", context, minimum=0)
        last = _integer(raw_tag, "to", context, minimum=first)
        if last >= frame_count:
            raise AsepriteFormatError(f"{context}.to is outside the frame list")
        direction = raw_tag.get("direction", "forward")
        if direction not in {"forward", "reverse", "pingpong", "pingpong_reverse"}:
            raise AsepriteFormatError(f"{context}.direction is not supported")
        repeat = raw_tag.get("repeat")
        if repeat is not None and (
            isinstance(repeat, bool) or not isinstance(repeat, int) or repeat < 0
        ):
            raise AsepriteFormatError(f"{context}.repeat must be a non-negative integer")
        if name in tags:
            raise AsepriteFormatError(f"duplicate animation tag {name!r}")
        tags[name] = AnimationTag(name, first, last, direction, repeat)
    return tags


def _rect(value: Mapping[str, Any], context: str) -> Rect:
    return Rect(
        _integer(value, "x", context, minimum=0),
        _integer(value, "y", context, minimum=0),
        _integer(value, "w", context, minimum=1),
        _integer(value, "h", context, minimum=1),
    )


def _size(value: Mapping[str, Any], context: str) -> Size:
    return Size(
        _integer(value, "w", context, minimum=1),
        _integer(value, "h", context, minimum=1),
    )


def _mapping(value: Mapping[str, Any], key: str, context: str) -> Mapping[str, Any]:
    child = value.get(key)
    if not isinstance(child, Mapping):
        raise AsepriteFormatError(f"{context}.{key} must be an object")
    return child


def _string(value: Mapping[str, Any], key: str, context: str) -> str:
    child = value.get(key)
    if not isinstance(child, str) or not child:
        raise AsepriteFormatError(f"{context}.{key} must be a non-empty string")
    return child


def _boolean(value: Mapping[str, Any], key: str, context: str) -> bool:
    child = value.get(key)
    if not isinstance(child, bool):
        raise AsepriteFormatError(f"{context}.{key} must be a boolean")
    return child


def _integer(
    value: Mapping[str, Any], key: str, context: str, *, minimum: int
) -> int:
    child = value.get(key)
    if isinstance(child, bool) or not isinstance(child, int) or child < minimum:
        raise AsepriteFormatError(
            f"{context}.{key} must be an integer greater than or equal to {minimum}"
        )
    return child


def _scale(value: Any) -> float:
    if isinstance(value, bool):
        raise AsepriteFormatError("meta.scale must be a positive number")
    try:
        scale = float(value)
    except (TypeError, ValueError) as error:
        raise AsepriteFormatError("meta.scale must be a positive number") from error
    if scale <= 0:
        raise AsepriteFormatError("meta.scale must be a positive number")
    return scale


__all__ = [
    "Animation",
    "AnimationFrame",
    "AnimationTag",
    "AsepriteFormatError",
    "Rect",
    "Size",
    "load_aseprite",
    "loads_aseprite",
    "parse_aseprite",
]
