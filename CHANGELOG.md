# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Unreleased

### Changed

- Improved logging and error handling

### Fixed

- Fixed bug where pyramid apex angle was set incorrectly based on crystal edge angle rather than face angle

## 3.1.0 - 2021-02-21

### Changed

- Improved logging and error messages in case shader compilation fails
- Population weight was changed from an integer to a decimal number for more granular control

## 3.0.0 - 2021-02-21

### Added

- Created a realistic sky and sun model based on Hosek-Wilkie and Preetham papers
- Simulation rate shown in status bar
- Ice crystals now have adjustable pyramidal caps
- Possibility to save and load simulations
- Added controls for prism face distances from the crystal C-axis
- Preview window for crystal shape and orientation
- Better logging and error handling
- Checkbox to enable or disable a crystal population

### Changed

- Forced application to use native desktop OpenGL instead of ANGLE or software renderer
- Build system switched to qmake from CMake
- Rewrote most of the GUI code to use Qt model/view architecture
- Triangle normals are now cached during raytracing
- The settings groups in the side panel are now collapsible
- Adjusting sliders with arrow kys now uses smaller steps
- Light is now allowed to bounce inside ice crystal for a 100 times instead of 10

### Fixed

- Fixed bug where intensity of halos changed with rays per frame setting
- Fixed a tiny raytracing artifact in the middle of the view

## 2.4.0 - 2019-07-29

### Added

- Show error dialog and exit if GPU does not support OpenGL 4.4

### Fixed

- Bug where changing multiple scattering probability did not trigger a new
  render if maximum iterations were reached
- Crashing on Intel GPUs with Linux and Mesa3D

## 2.3.1 - 2019-07-14

### Fixed

- Build configuration bug where resources were not being linked on Linux

## 2.3.0 - 2019-07-13

### Added

- Added menu bar with option to save simulation result as image
- Added slider for setting the probability of simple double scattering

### Changed

- Replaced Add Population and Remove Population texts with icons

## 2.2.0 - 2019-07-09

### Added

- Created an application icon for HaloRay
- Added possibility to add preset crystal populations

### Changed

- Disabled "Remove population" button when there is only one population in simulation
- Added multiple crystal populations by default

## 2.1.1 - 2019-07-08

### Fixed

- Fixed bug where removing the only crystal population caused the UI controls not to work anymore

## 2.1.0 - 2019-07-07

### Added

- Gave main window an initial size hint
- Support for multiple crystal populations

### Changed

- Made sidebar scrollable when the window is small enough
- Made UI show field of view in degrees
- Limited field of view for stereographic, rectilinear and orthographic projections

### Fixed

- Fixed bug where field of view sometimes did not update correctly when choosing a new projection

## 2.0.0 - 2019-07-02

### Changed

- Replaced Nuklear UI with Qt

## 1.1.0 - 2019-06-17

### Changed

- Renamed C-axis orientation to simply C-axis tilt

### Fixed

- Fixed bug where uniform C-axis orientation was messing with C-axis rotation

## 1.0.0 - 2019-06-16

- First official release
