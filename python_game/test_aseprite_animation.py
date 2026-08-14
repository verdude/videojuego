import copy
import json
import tempfile
import unittest
from pathlib import Path

from aseprite_animation import AsepriteFormatError, load_aseprite, parse_aseprite


EXAMPLE = {
    "frames": {
        "ghost 0.aseprite": {
            "frame": {"x": 0, "y": 0, "w": 7, "h": 11},
            "rotated": False,
            "trimmed": True,
            "spriteSourceSize": {"x": 4, "y": 2, "w": 7, "h": 11},
            "sourceSize": {"w": 16, "h": 16},
            "duration": 150,
        },
        "ghost 1.aseprite": {
            "frame": {"x": 7, "y": 0, "w": 7, "h": 11},
            "rotated": False,
            "trimmed": True,
            "spriteSourceSize": {"x": 4, "y": 2, "w": 7, "h": 11},
            "sourceSize": {"w": 16, "h": 16},
            "duration": 250,
        },
        "ghost 2.aseprite": {
            "frame": {"x": 14, "y": 0, "w": 7, "h": 11},
            "rotated": False,
            "trimmed": True,
            "spriteSourceSize": {"x": 4, "y": 2, "w": 7, "h": 11},
            "sourceSize": {"w": 16, "h": 16},
            "duration": 200,
        },
    },
    "meta": {
        "image": "ghost.png",
        "size": {"w": 21, "h": 11},
        "scale": "1",
        "frameTags": [],
    },
}


class ParseAsepriteTests(unittest.TestCase):
    def test_parses_example_metadata(self):
        animation = parse_aseprite(EXAMPLE)

        self.assertEqual(animation.image, "ghost.png")
        self.assertEqual(animation.sheet_size.tuple, (21, 11))
        self.assertEqual(animation.duration_ms, 600)
        self.assertEqual(animation.current_frame.name, "ghost 0.aseprite")
        self.assertEqual(animation.current_frame.atlas_rect.tuple, (0, 0, 7, 11))
        self.assertEqual(animation.current_frame.source_position, (4, 2))
        self.assertEqual(animation.current_frame.source_size.tuple, (16, 16))

    def test_uses_each_frames_duration_and_loops(self):
        animation = parse_aseprite(EXAMPLE)

        self.assertEqual(animation.update(149).name, "ghost 0.aseprite")
        self.assertEqual(animation.update(1).name, "ghost 1.aseprite")
        self.assertEqual(animation.update(250).name, "ghost 2.aseprite")
        self.assertEqual(animation.update(200).name, "ghost 0.aseprite")

    def test_large_update_can_cross_multiple_frames(self):
        animation = parse_aseprite(EXAMPLE)

        self.assertEqual(animation.update(760).name, "ghost 1.aseprite")
        self.assertEqual(animation.elapsed_ms, 10)

    def test_complete_cycles_are_skipped(self):
        animation = parse_aseprite(EXAMPLE)

        self.assertEqual(animation.update(600_000_150).name, "ghost 1.aseprite")
        self.assertEqual(animation.elapsed_ms, 0)

    def test_creates_independent_tag_player(self):
        data = copy.deepcopy(EXAMPLE)
        data["meta"]["frameTags"] = [
            {"name": "vanish", "from": 0, "to": 2, "direction": "reverse"}
        ]
        animation = parse_aseprite(data).for_tag("vanish", loop=False)

        self.assertEqual(animation.current_frame.name, "ghost 2.aseprite")
        animation.update(200)
        self.assertEqual(animation.current_frame.name, "ghost 1.aseprite")
        animation.update(400)
        self.assertEqual(animation.current_frame.name, "ghost 0.aseprite")
        self.assertFalse(animation.playing)

    def test_loads_json_file(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "ghost.json"
            path.write_text(json.dumps(EXAMPLE), encoding="utf-8")

            animation = load_aseprite(path)

        self.assertEqual(len(animation.frames), 3)

    def test_reports_invalid_duration_with_context(self):
        data = copy.deepcopy(EXAMPLE)
        data["frames"]["ghost 0.aseprite"]["duration"] = 0

        with self.assertRaisesRegex(AsepriteFormatError, r"frames\[0\]\.duration"):
            parse_aseprite(data)


if __name__ == "__main__":
    unittest.main()
