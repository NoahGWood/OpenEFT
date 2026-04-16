# MVP TODO

## Phase 1

### Core Workflow
- [x] Load AN2/EFT File
- [ ] Edit AN2/EFT File
- [x] Validate AN2/EFT File
- [x] Save AN2/EFT File

### Commands
- [x] Item
  - [x] Add
  - [x] Edit
  - [x] Delete

- [x] Subfield
  - [x] Add
  - [x] Edit
  - [x] Delete

- [x] Field
  - [x] Add
  - [x] Edit
  - [x] Delete

- [x] Record
  - [x] Add
  - [x] Edit
  - [x] Delete


### UI Foundation
- [x] File Selector (List of available files)
- [x] File Hierarchy Panel (Viewer)
- [ ] Inspector (Editing Panel)
  - [ ] Text
  - [ ] Enum
  - [x] Binary/Raw
  - [x] Binary/Image 


### Transaction Wizard
- [ ] Auto-Build TX Forms from TX Rules

## Phase 2
### Rules Engine
- [x] Load EBTS Rules Profile
- [x] Edit EBTS Rules Profile
- [ ] Save EBTS Rules Profile
  - [ ] Implement Enum Registry hook in ValueConstraint export


### Validation
- [ ] Run validation against AN2/EFT file
  - [ ] Implement Enum Registry usage in Validator for ValueConstraints
- [x] Display useful errors
- [x] Export validation results

## Phase 3
### FD Scanning
- [ ] Load FD-258 scanned image
- [ ] Extract FP regions
- [ ] Viewer
- [ ] Inspector
- [ ] Manual alignment controls

## Phase 4
### FP Records
- [ ] NFIQ2 Quality Scoring
- [ ] WSQ Encoding (type 4)
- [ ] JP2k Encoding (type 14)

## Phase 5
### Default CJIS Profile
- [ ] FAUF TX
- [ ] DOM, TOT, POB, CTZ, etc.