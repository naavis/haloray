# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Unreleased

### Added
- Created an application icon for HaloRay
- Added possibility to add preset crystal populations

### Changed
- Disabled "Add population" button when there is only one population in simulation
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
