# Aseprite animation parser

`aseprite_animation.py` turns Aseprite's JSON hash export into typed frame
metadata and a time-based animation player:

```python
import pygame as pg

from aseprite_animation import load_aseprite

animation = load_aseprite("data/ghost.json")
sprite_sheet = pg.image.load(f"data/{animation.image}").convert_alpha()

# In the game loop, where clock is a pygame.time.Clock:
animation.update(clock.tick(60))
frame = animation.current_frame
frame_image = sprite_sheet.subsurface(frame.atlas_rect.tuple)
screen.blit(frame_image, frame.source_position)
```

The player uses each frame's exported duration, loops by default, and supports
named Aseprite frame tags through `animation.for_tag("tag-name")`. The frame's
`source_position` and `source_size` preserve the placement information needed
to render trimmed exports without jitter.
