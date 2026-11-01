# Videojuego

This is a videogame (videjuego) that currently has only 1 mode but will have multiple modes in the
future.

## Scenes

A scene is a game mode. It will contain a main loop that runs the game.

The Combat Mode loop is currently in main.cpp#main but will be moved to src/combat/scene.hpp when other
modes are added and the game will swap between loops for each scene.

## Combat Mode

A 2d fighting game.

### Loop

This loop in particular operates on a fixed timestep of 60 ticks per second. About 16.667ms between
each tick. The simulation is updated only if enough time has accrued. Multiple ticks will run if
enough time has accrued - this is indicative of poor performance.

### Input

Inputs are collected in the loop from SDL and converted into in-game commands. This bit is
currently in development.

### Moves

The goal is to have lots of variety in movesets for the characters, with full tekken-style combos.

Every move will have multiple phases. For example:

- startup
- active
- recovery

Only the `active` phase will contains hitboxes that do damage. An AnimationClip
(src/animation/animationclip.hpp) is the full animation for the move and will be rendered at 1
frame per simulation tick. The `MoveState` class in `src/combat/move.hpp` keeps track of which frame
should be rendered in the current tick. It is held by the `Fighter` in `src/combat/fighter.hpp`.

Active phases are currently deduced by the presence of a hitbox.
Need to determine how to label the animations with additional metadata, such as:
- frames during which input is accepted
- cancellable frames and inputs
- combo sequences

## Graphics

`src/graphics` should serve as the boundary between the game code and the SDL3 library (aside from
input handling). Keeping SDL isolated and abstracted will make transitioning to other graphics
libraries like vulkan easier in the future.

### TextureStore/Graphics

The TextureStore simply holds loaded SDL Textures, which are GPU render-able versions of a
spritesheet. This can be swapped with other libraries' representations in the future with
minimal effort due to the abstraction around an `SDL_Texture*`.

`src/graphics/graphics.hpp` contains a class to hold and manage the rendering strategy separately
from the rest of the program.

### Animation

Moves are strongly tied to their animation, which defines the core of their behavior.
A `MoveDefinition` is created for each tag in the aseprite export.

Aseprite is the source of truth and the game engine relies on the export metadata to determine
hitboxes, hurtboxes and animations. There is a fork of aseprite available that allows creation of
hitboxes and hurtboxes directly on each frame here:
[https://github.com/verdude/aseprite](https://github.com/verdude/aseprite).

Aseprite export pairs in `assets/\*.(json|png)` are loaded at startup. The convention is to make
1 spritesheet per character. This may change when additional modes are introduced.

`src/animation/assetstore.hpp` loads and owns all of the loaded spritesheets. The current strategy
is to load all spritesheets at startup and destroy them all when the program exits.
`MoveDefinitions` reference AnimationClips that are loaded in a `SpriteSheet`.

`src/animation/spritesheet.hpp` parses the animations from the aseprite json export into
AnimationClips which contain the full sequence of frames for that specific animation. Aseprite tags
are used to specify the beginning and end of an animation. The name of the tag corresponds to the
name of the move/action that an entity can take. The name of the spritesheet corresponds to the name
of the in-game entity.

## Plans in order of priority

- input handling
    - tekken-esque combos (more moves and engine support)
- collision detection
- adventure mode
    - engine support
    - story design
    - swap to combat mode triggers and detailed result stats
- more characters
- Combat AI
- character select
- music
- multiplayer fights: netcode with rollback
