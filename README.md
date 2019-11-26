# Metroid Prime Practice Mod Patcher
This project can produce a release & patch a metroid prime iso to be the practice mod

## How to install (Windows)

- Get a metroid prime .iso (i.e. using a hacked wii and cleanrip)
- Copy a metroid prime iso to 'prime.iso in this directory'
- Run `patch.bat`

## How to install (Non-windows)
- Get a metroid prime .iso (i.e. using a hacked wii and cleanrip)
- Copy a metroid prime iso to 'prime.iso in this directory'
- Run `patch.sh`

## Usage
- Launch the iso in Dolphin
- Copy to your wii, and launch using Nintendont
  - If your Nintendont is really old, update Nintendont
  - Note this mod is not currently compatible with some settings, at the very least:
     - Cheats
     - Force progressive
     - Force Widescreen
  - There may be others, not a lot of testing has been done yet
  
## Controls
`start` - Open menu
- `up`/`d-pad up`/`c up` - Up one item
- `down`/`d-pad down`/`c down` - Down one item
- `left`/`d-pad left`/`c left` - Up 5 items
- `right`/`d-pad right`/`c right` - Down 5 items
- `l digital` - To top/min
- `r digital` - To bottom/max
- `a` - Select item in menu
- `b` - Go back a menu

## Known Issues
- Soft reset (`b + x + start`) crashes in Nintendont
- Non-menu text is garbled
- Rare crashes (so far, inconsistent)
- Warps to a couple rooms will put you out of bounds


## Menus
- Inventory
  - Edit your item inventory
  - Artifacts do nothing in your inventory, and must be collected to have any effect, and so are not current editable
- Player
  - Save/load position
  - Edit position
  - Toggle Floaty Jump
  - Player state information (careful editing this!)
- Warp
  - Warp to any room
- Config
  - Adjust which HUD elements appear

## HUD
- Input
  - Current IGT
  - Digital Inputs
  - Analog Main Stick
- Player Speed
  - Current speed (x, y, z, horizontal total)
  - Currrent rotational speed (x, y, z, total)
- Room timers
  - Current room IGT, in seconds
  - Current room IGT, in frames
  - Previous room IGT, in seconds
  - Previous room IGT, in frames
- FPS
  - Game-measured FPS

## Changelog

### 1.2.5
- Fix warps, so they warp you to the destination, instead of the credits

### 1.2.4
- Remove ZOOM from next to the IGT on save select
- Allow IGT and input to be configured separately

### 1.2.3
- Add position/precise position displays
- Change file select to show mm:ss instead of hh:mm

### 1.2.2
- Rewrote patcher to native (much smaller, faster)

### 1.2.1
- IS enabler (requires morph)
- Lightshow Fix (requires unmorphed)
- Update patcher
- May have fixed soft-reset crash

### 1.2.0
- Add trigger rendering

### 1.1.2
- Minor performance improvements
- Internal rework of how native is built

### 1.1.1
- Minor fixes

### 1.1.0
- Add Player menu
- Add FPS counter

### 1.0.0
 - Initial Release
