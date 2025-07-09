## [3.20.1](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.20.0...v3.20.1) (2025-07-09)


### Bug Fixes

*  read_reg_as_temperature now returns NAN ([18d7799](https://github.com/UCSD-E4E/smartfin-fw3/commit/18d7799c997362eb0005f8d266129682dd904cf9))
* Fixes includes ([a769e7f](https://github.com/UCSD-E4E/smartfin-fw3/commit/a769e7ffe59112e089448bd5f2fccbfd8fe3ff1f))
* read_reg_as_temperature returns NaN on invalid register ([cb3fe67](https://github.com/UCSD-E4E/smartfin-fw3/commit/cb3fe670ff542b41f71195958c24888c49d286d2))

# [3.20.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.19.0...v3.20.0) (2025-07-09)


### Bug Fixes

* Adds sync functions to PC build ([2afba18](https://github.com/UCSD-E4E/smartfin-fw3/commit/2afba18bb93fcd8cad558b7f12f5526da9c3cb2f))


### Features

* Adds time sync ([16ae894](https://github.com/UCSD-E4E/smartfin-fw3/commit/16ae894cd89a5eb7647fbe85a5083610643f8bf2))
* Adds transfer mode ([22954b9](https://github.com/UCSD-E4E/smartfin-fw3/commit/22954b918e4ddd73684fa8a02dc10eaaf83f1fa1))

# [3.19.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.18.0...v3.19.0) (2025-06-26)


### Bug Fixes

* Inlines build date/time ([19bfc35](https://github.com/UCSD-E4E/smartfin-fw3/commit/19bfc354da0f2b95cb634cb4a860bca4065cc7dd))


### Features

* Adds GPS to monitor sensors ([5af686b](https://github.com/UCSD-E4E/smartfin-fw3/commit/5af686bf7aa0993136968b32b5957bdd1a8a950d))

# [3.18.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.17.4...v3.18.0) (2025-06-25)


### Features

* Implements the manufacturing test ([f616103](https://github.com/UCSD-E4E/smartfin-fw3/commit/f616103664d3a6514222f2ee69a98fe6bfb5ff22))
* Implements the manufacturing test ([#254](https://github.com/UCSD-E4E/smartfin-fw3/issues/254)) ([5e80666](https://github.com/UCSD-E4E/smartfin-fw3/commit/5e806669fa5dbd3abf94ac6fbb42a5d13a7a6dad))

## [3.17.4](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.17.3...v3.17.4) (2025-06-19)


### Bug Fixes

* Adds isValid ([bceca15](https://github.com/UCSD-E4E/smartfin-fw3/commit/bceca15451dfe06a25098fb985c41a1fbf2d4c7d))
* Adds upload count to flog ([6d203f3](https://github.com/UCSD-E4E/smartfin-fw3/commit/6d203f3c55fff60fd1e70abc575d6feb961d7f7e))
* Disables timers on deinit ([957a47d](https://github.com/UCSD-E4E/smartfin-fw3/commit/957a47ddfce834d28dd34b2f158ea8af6508495f))
* Sets session time based on RTC ([e30b757](https://github.com/UCSD-E4E/smartfin-fw3/commit/e30b75744dcec6499d8180b50f8ad1adfc151cd5))

## [3.17.3](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.17.2...v3.17.3) (2025-06-18)


### Bug Fixes

* Adds summary display ([55d77cc](https://github.com/UCSD-E4E/smartfin-fw3/commit/55d77cc3e5117e104d80ba3eb412af811cbb886a))
* Fixes accumulate reset ([86ea71d](https://github.com/UCSD-E4E/smartfin-fw3/commit/86ea71d3885c81588e8ac36aad8d0e56f4a43f05))

## [3.17.2](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.17.1...v3.17.2) (2025-06-13)


### Bug Fixes

* Fixes LED behaviors ([1a69495](https://github.com/UCSD-E4E/smartfin-fw3/commit/1a6949549a0a1fff3dc0009cae25d909e0a2825e))
* Fixes LED behaviors ([#251](https://github.com/UCSD-E4E/smartfin-fw3/issues/251)) ([abd8693](https://github.com/UCSD-E4E/smartfin-fw3/commit/abd86936e9b1ca4c574999bbd13bf85e46234910))
* Inhibits upload and enables high data rate ([cdaa1e1](https://github.com/UCSD-E4E/smartfin-fw3/commit/cdaa1e19b1ffd24c00e1cbca1e4534d72720f082))

## [3.17.1](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.17.0...v3.17.1) (2025-06-13)


### Bug Fixes

* Uses correct printf fn ([6cd3fc5](https://github.com/UCSD-E4E/smartfin-fw3/commit/6cd3fc5d09979add759a16ed9df3c824b3766667))

# [3.17.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.16.0...v3.17.0) (2025-06-11)


### Bug Fixes

* Adds platform enum ([c1b4df0](https://github.com/UCSD-E4E/smartfin-fw3/commit/c1b4df0a96600de4ec7ba6468bb8bb59d958952d))


### Features

* Adds new flog codes ([b842e03](https://github.com/UCSD-E4E/smartfin-fw3/commit/b842e0385dc4597acc83c297b6941b3b178cec22))
* Adds state logging ([4a91130](https://github.com/UCSD-E4E/smartfin-fw3/commit/4a91130dc068f689bbb14d4c0395b3392fb7fa5a))
* Moves initialization to thread ([60ce804](https://github.com/UCSD-E4E/smartfin-fw3/commit/60ce8043a9af054669a1ee1bfa179a4e289d774e))

# [3.16.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.15.0...v3.16.0) (2025-06-11)


### Bug Fixes

* Adds platform guards ([6121cae](https://github.com/UCSD-E4E/smartfin-fw3/commit/6121cae9af506012dd3a84a08fde7d785a8000ba))


### Features

* Quits IMU thread prior to sleep ([72dc5d2](https://github.com/UCSD-E4E/smartfin-fw3/commit/72dc5d2fdfb49c7d712d92f3fc1e355e4ed3e61b))
* Quits IMU thread prior to sleep ([#248](https://github.com/UCSD-E4E/smartfin-fw3/issues/248)) ([d0d64fb](https://github.com/UCSD-E4E/smartfin-fw3/commit/d0d64fbe552f0b45a969e5a229874317fb4452cd))

# [3.15.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.14.2...v3.15.0) (2025-06-09)


### Bug Fixes

* Added error handling through error codes for init_file_mapping and resize_file. Added flag for active status of conioHistory module to ensure that program can still run smoothly after an error with file mapping ([b34437f](https://github.com/UCSD-E4E/smartfin-fw3/commit/b34437fe0e554deeb6b9a54cd2a1e7f67fdbca0d))
* Added file write history implementation to conio.cpp ([238a2f2](https://github.com/UCSD-E4E/smartfin-fw3/commit/238a2f291e212910e35dd77e64052c61319c9497))
* Added minimal functionality to detect scroll up and down ([6afe85b](https://github.com/UCSD-E4E/smartfin-fw3/commit/6afe85bbacd46a0991031ab0f5fda331997fa151))
* Change file size increase to linear step of 32MB upon reaching 32MB size ([5d849b4](https://github.com/UCSD-E4E/smartfin-fw3/commit/5d849b4c0b1d761d827b619022ae5f7a91246e13))
* Changed decimal values to char representation when checking user input ([6ecc2c3](https://github.com/UCSD-E4E/smartfin-fw3/commit/6ecc2c3c74896e2ba71da70c8744f2547bfdb274))
* Changed format string from C++ string to C string for overwriting with vsnprintf to avoid undefined behavior ([b014908](https://github.com/UCSD-E4E/smartfin-fw3/commit/b014908cf40a728523d7b4c4bbae3e398f832297))
* Changed SF_OSAL_printf logic to match behavior of Particle implementation ([8c90603](https://github.com/UCSD-E4E/smartfin-fw3/commit/8c90603c722f128dc72179719bf42a602c9b5aa0))
* Changed size_t to int64_t because possible_frag can be negative ([addc6e0](https://github.com/UCSD-E4E/smartfin-fw3/commit/addc6e01128e2644467410d13f247bf67731b076))
* Created helper module, conioHistory.cpp, for conio.cpp to save history. ([5dd355f](https://github.com/UCSD-E4E/smartfin-fw3/commit/5dd355fcb9911099cbb3e7ae7e58b7a3af2f88c8))
* Fixed offset alignment of new Lines and ensured retrieved lines are properly null-terminated and checked if empty ([58af57b](https://github.com/UCSD-E4E/smartfin-fw3/commit/58af57b813a97969294c67ae404f2172f1af42f6))
* Have conioHistory start off as inactive and become active upon successful init_file_mapping. All error handling deactivates conioHistory ([1778e57](https://github.com/UCSD-E4E/smartfin-fw3/commit/1778e571867d9ae3517d38c6c61503c04756f7c9))
* Implemented new line logging for conio ([c71e170](https://github.com/UCSD-E4E/smartfin-fw3/commit/c71e170396292f4ad55db0ddf4de9dd1830d7de1))
* Included guard to only have CLI history apply to pc_build. ([5e6c0bc](https://github.com/UCSD-E4E/smartfin-fw3/commit/5e6c0bc2d9d5cd264a83baf64abe837ef27b72c7))
* Modified SF_OSAL_getch to consume invalid characters and block until next character. Scroll logic is moved to getch since scroll inputs are considered invalid characters to be returned by getch ([1759b47](https://github.com/UCSD-E4E/smartfin-fw3/commit/1759b4709ebea9e6981cc20e5bc41c933934b2fd))
* Modified write_line to support both display and non-display lines ([124d145](https://github.com/UCSD-E4E/smartfin-fw3/commit/124d1459dbb1f7a30d2e6c7fbf7e50d3bb26e59b))
* Moved scroll logic to read loop ([28cf234](https://github.com/UCSD-E4E/smartfin-fw3/commit/28cf2348a5ba73a9cb8359c8d86c7bfc44e20a81))
* Refactored conioHistory to better reflect history metadata and function like architecture specification ([7a94de5](https://github.com/UCSD-E4E/smartfin-fw3/commit/7a94de5b299f73efb1a9ed39b35db74ee7693a34))
* Reimplemented retrieve_display_line (formerly retrieve_line) to only retrieve display lines and their fragments. Relevant documentation updated ([eb9e49e](https://github.com/UCSD-E4E/smartfin-fw3/commit/eb9e49ec5af68a6cc32b3bc17fcd4a366280f664))
* Restructured CONIO_hist_line to begin supporting non-display lines. CPP file reflects new constructor ([d3a9bcc](https://github.com/UCSD-E4E/smartfin-fw3/commit/d3a9bcc7c6e13099365ce1ac8fd70569842ddd05))
* Restructured getline to separately handle scroll input (important for UX concerning user input during scrolling) and ensured that exceeded size inputs are considered entered as a command ([650ede7](https://github.com/UCSD-E4E/smartfin-fw3/commit/650ede7d2848bf029a837183e6cc7f4f443f139b))
* Scrolling for CLI pc_build ([#184](https://github.com/UCSD-E4E/smartfin-fw3/issues/184)) ([76e7910](https://github.com/UCSD-E4E/smartfin-fw3/commit/76e7910a3ef08c8b77bbb55a1a31098531a76eb6)), closes [#166](https://github.com/UCSD-E4E/smartfin-fw3/issues/166)
* SF_OSAL_getch now blocks until there is a new input ([f216d4f](https://github.com/UCSD-E4E/smartfin-fw3/commit/f216d4f356f6939efc25190df5964942218602be))
* TEMPORARY - log will stop writing after reaching max file size. ([c8920a8](https://github.com/UCSD-E4E/smartfin-fw3/commit/c8920a853bcccc11d437d3485237b9b6c044bbe7))
* Update the position of the line for fragmented display lines ([0a297f2](https://github.com/UCSD-E4E/smartfin-fw3/commit/0a297f26e159bac43a342d4d3c1c44f0d92b9da8))
* Updated module to retrieve lines from history and fixed how variables update for tracking history ([8ed00fc](https://github.com/UCSD-E4E/smartfin-fw3/commit/8ed00fc5b568722e12591dee6ea986ca0576108b))
* User input is updated into history log when scrolling ([6a3e5a3](https://github.com/UCSD-E4E/smartfin-fw3/commit/6a3e5a32219194a247d4d7f7fbea7672d5f46bb5))
* User input will bring the cli window back to the bottom if in the middle of scrolling ([a40150c](https://github.com/UCSD-E4E/smartfin-fw3/commit/a40150c3fbc4f8f1d3cda0cdfe5cdff01c73dae5))


### Features

* Added valgrind into Dockerfile ([c4041be](https://github.com/UCSD-E4E/smartfin-fw3/commit/c4041be2021bd572b4af07ab5330f1968d1aad8a))

## [3.14.2](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.14.1...v3.14.2) (2025-06-06)


### Bug Fixes

* Delays water sensor initialization to setup ([15a0aa8](https://github.com/UCSD-E4E/smartfin-fw3/commit/15a0aa8bfd9a89e67bcdf70c12aeffc59bda50c4))
* Delays water sensor initialization to setup ([#244](https://github.com/UCSD-E4E/smartfin-fw3/issues/244)) ([edd1fb2](https://github.com/UCSD-E4E/smartfin-fw3/commit/edd1fb2e3b54efe60d99d1f98e6b445be1bc3464))

## [3.14.1](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.14.0...v3.14.1) (2025-06-05)


### Bug Fixes

* Adds locks to I2C accesses ([7243187](https://github.com/UCSD-E4E/smartfin-fw3/commit/7243187a4a750d2540b7e4bc6c317adae71cc47d))
* Adds platform dependent block ([d8c493a](https://github.com/UCSD-E4E/smartfin-fw3/commit/d8c493aca0a0ab05b374ed988a84877f37de5079))
* Delays scheduler init ([49d62a8](https://github.com/UCSD-E4E/smartfin-fw3/commit/49d62a87c90c686fa0ec8dc18722536d282a4044))
* Enables HDR ensembles ([b9fa6c4](https://github.com/UCSD-E4E/smartfin-fw3/commit/b9fa6c496859111f6e4cd4829b0f72a1adcb16e4))
* FIxes buffer overflow and direntry stack ([67dd75a](https://github.com/UCSD-E4E/smartfin-fw3/commit/67dd75af03b8f6937ec24439c3c049cd217368fc))
* Fixes delay computation ([ca4755f](https://github.com/UCSD-E4E/smartfin-fw3/commit/ca4755f129cb2520179d7cba9f3281a6915ee244))
* Fixes initialization ([23153ef](https://github.com/UCSD-E4E/smartfin-fw3/commit/23153ef990435daa13a35c3964c0640bcb7f14ce))
* Rename shadowed variable ([1235b2e](https://github.com/UCSD-E4E/smartfin-fw3/commit/1235b2ec3742fe88261f344e87c086b2f45b2d82))
* Reorders schedule ([e32f15a](https://github.com/UCSD-E4E/smartfin-fw3/commit/e32f15acf30c72458df18d27c869a124d1cfe5ad))
* Sets session start time ([29fcc7a](https://github.com/UCSD-E4E/smartfin-fw3/commit/29fcc7a319bf4fc6b3b265ea947c4b2ada969e24))
* Switches to dynamic FileCLI object ([3e95335](https://github.com/UCSD-E4E/smartfin-fw3/commit/3e95335977228c6a3e84c6c0ad52704fd977dae5))

# [3.14.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.13.0...v3.14.0) (2025-06-04)


### Bug Fixes

* Adds deploy time display ([bc7f351](https://github.com/UCSD-E4E/smartfin-fw3/commit/bc7f351e1a5c2951789ba69ad92acd8298bc6ec7))
* Allows dynamic resize ([a401eaa](https://github.com/UCSD-E4E/smartfin-fw3/commit/a401eaac8bae08191996ad1ed57bced182923697))
* Allows on the fly resize ([7124f13](https://github.com/UCSD-E4E/smartfin-fw3/commit/7124f1307deafdee44479bf9364a1fa14a02bff5))
* Fixes getline return value ([7299d4b](https://github.com/UCSD-E4E/smartfin-fw3/commit/7299d4b871386f01a34e04d90ed219ef3d17f52d))
* Fixes window size variable ([5bbb008](https://github.com/UCSD-E4E/smartfin-fw3/commit/5bbb008505e658a250afd9afbf25be4b19779395))
* Initializing window size from NVRAM ([8e98621](https://github.com/UCSD-E4E/smartfin-fw3/commit/8e98621d34328557bfc1447b4cb7b436a270d973))
* Lower limit ([3577ac7](https://github.com/UCSD-E4E/smartfin-fw3/commit/3577ac744e5aa88ddbd7b136a135fde504774940))
* Window array ([04d1838](https://github.com/UCSD-E4E/smartfin-fw3/commit/04d18388bb9dbfee86ddc3a74901a0d2ad2dab74))


### Features

* Adds command for setting window length ([f5ba3f1](https://github.com/UCSD-E4E/smartfin-fw3/commit/f5ba3f106d2cee825a644f983f5df96f153b5d31))

# [3.13.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.12.1...v3.13.0) (2025-06-04)


### Bug Fixes

* Added read loop ([774fda3](https://github.com/UCSD-E4E/smartfin-fw3/commit/774fda3c00f8c5c948bace0f6beec1b54b6631f1))
* Adds binary display ([cddb618](https://github.com/UCSD-E4E/smartfin-fw3/commit/cddb61808a05c954bc5d933cd195328a3f381fc0))
* Adds CLI linkage ([2b1cbf7](https://github.com/UCSD-E4E/smartfin-fw3/commit/2b1cbf77a7b0753516a64257ef4621cf4485b35f))
* Adds compass ([d797f77](https://github.com/UCSD-E4E/smartfin-fw3/commit/d797f77c27affb121b39c9c2b719c3dd0f4109fb))
* Adds enum for headers ([ea40db0](https://github.com/UCSD-E4E/smartfin-fw3/commit/ea40db01fd930dc469b892530ce18767bbe905c9))
* Adds FLOG handy to CLI ([5589661](https://github.com/UCSD-E4E/smartfin-fw3/commit/55896610e34c62d5de1fdee2b9e4fabcb52a83d6))
* Adds mag regs ([b0e1768](https://github.com/UCSD-E4E/smartfin-fw3/commit/b0e17682acbc25ed17abe93ec15f1e5d838776d0))
* Adds space for readability ([268e129](https://github.com/UCSD-E4E/smartfin-fw3/commit/268e129fa680b5ea9f53ce7423c7e6da0d0c54fb))
* Changes quat computation ([d6b7469](https://github.com/UCSD-E4E/smartfin-fw3/commit/d6b7469634a64d41783ddd5f944a66d95bfe5f2f))
* Consolidates initialization ([cf31ec2](https://github.com/UCSD-E4E/smartfin-fw3/commit/cf31ec24eb18a1be6a0c39208c867136867081c9))
* Enables DMP/FIFO, fixes samplerate ([3b28dc4](https://github.com/UCSD-E4E/smartfin-fw3/commit/3b28dc4e3cd4d1d9958288bdf6e75c36417db142))
* Enables gyroscope ([816ffa6](https://github.com/UCSD-E4E/smartfin-fw3/commit/816ffa60186ad777246d87c3946e5c5f334ca9c9))
* Fixes boot loop trap ([13b2056](https://github.com/UCSD-E4E/smartfin-fw3/commit/13b20560d0b4d0fca0e8770e134fd676d1ef3f03))
* Fixes pc_hal ([db4e876](https://github.com/UCSD-E4E/smartfin-fw3/commit/db4e87696a2dfad69c78213566c4e82771fde2bd))
* Fixes redefinition ([8fb6ac0](https://github.com/UCSD-E4E/smartfin-fw3/commit/8fb6ac03c168029fa76486226084f670eee6a5d1))
* Fixes scaling ([5e3a33d](https://github.com/UCSD-E4E/smartfin-fw3/commit/5e3a33d0c54a251ccf0b1c9403956cb4b32c9c5e))
* Fixes stack size definitions ([6eab186](https://github.com/UCSD-E4E/smartfin-fw3/commit/6eab186b90598d5907a174d71df6920b97704efa))
* Fixes Thread definition ([1ac33b0](https://github.com/UCSD-E4E/smartfin-fw3/commit/1ac33b0f1e0965ef2fcc37d1854d285a8cd22118))
* Fixes weak linkage ([dc9b029](https://github.com/UCSD-E4E/smartfin-fw3/commit/dc9b029fdbf5145cabed88ecd2bbd938f27f5c74))
* Override driver DMP initialization ([b22c7da](https://github.com/UCSD-E4E/smartfin-fw3/commit/b22c7da73b0368718180a816d7636e0536d5eedb))
* Reenables DMP data monitor ([ddbde12](https://github.com/UCSD-E4E/smartfin-fw3/commit/ddbde12412d4886f0e79322aacbd268a9338cad5))
* Reenables normal AGMT access ([c72b30d](https://github.com/UCSD-E4E/smartfin-fw3/commit/c72b30dd47923f1f55ffe7ce9fd982fa0bfde9a0))
* Removes implementation from pc_build ([b13a090](https://github.com/UCSD-E4E/smartfin-fw3/commit/b13a090767107ed1989cb4283fe25350d852864c))
* Removes obsolete function ([6d003e4](https://github.com/UCSD-E4E/smartfin-fw3/commit/6d003e4704db16ebd24537dd32f00d905bfe094c))
* Set ACCEL scale to 8g ([e8935e2](https://github.com/UCSD-E4E/smartfin-fw3/commit/e8935e22d5cf73c05856fd9cac850ef618733740))
* Set lpf bandwidth ([eaa909e](https://github.com/UCSD-E4E/smartfin-fw3/commit/eaa909e61052d8d2af090e9cf93269359ae093c2))
* Set ODR ([a39db4c](https://github.com/UCSD-E4E/smartfin-fw3/commit/a39db4cacc41375f122d3c8d24dfb44c60db8c61))
* Sets ACC and GYR cycled mode ([01a52c4](https://github.com/UCSD-E4E/smartfin-fw3/commit/01a52c44b2b41af71aabf49748e19d7bde258b2f))
* Switches to dynamic memory for thread primitives ([4563f2b](https://github.com/UCSD-E4E/smartfin-fw3/commit/4563f2bb5f50078232dc4f05dbf666cd9b101936))
* Switches to new headers ([0696265](https://github.com/UCSD-E4E/smartfin-fw3/commit/0696265c1df8f160ad71c1a793d7153be9dc97ea))
* Switches to new implementation ([686c684](https://github.com/UCSD-E4E/smartfin-fw3/commit/686c68447c19f9941e630972715d6a729cb9d08a))
* Undoes exposure of internal _device ([32a960b](https://github.com/UCSD-E4E/smartfin-fw3/commit/32a960b167f7212dbb0935598e1c7dfed4ffc652))
* Unhooks old IMU ([122f5b7](https://github.com/UCSD-E4E/smartfin-fw3/commit/122f5b737c2d14535c0806a2276b4c8ff7d9bc81))


### Features

* Adds Ens12 ([d8a065c](https://github.com/UCSD-E4E/smartfin-fw3/commit/d8a065c4d981fb826d7bbb169bd7bcdea706acc7))
* Adds panic prevention ([6295a64](https://github.com/UCSD-E4E/smartfin-fw3/commit/6295a64c0e5601368ace0b2b09fee446ef893ad7))
* Adds reg dump command ([4613bc0](https://github.com/UCSD-E4E/smartfin-fw3/commit/4613bc081f87bf40c4b8a9695fa1f2321407b366))
* Adds SF_INHIBIT_UPLOAD ([100c202](https://github.com/UCSD-E4E/smartfin-fw3/commit/100c20297bd8ca6eaeb6218382287514fd8b63ef))
* Exposes _devices ([1da7674](https://github.com/UCSD-E4E/smartfin-fw3/commit/1da7674519c34e9e3bb829ee6f16a428f75dd2dc))
* Switches to main thread ([4655a92](https://github.com/UCSD-E4E/smartfin-fw3/commit/4655a9239fd8e047eea71ee30cdfd2e5829fe434))
* Switches to new IMU implementation ([c2deb32](https://github.com/UCSD-E4E/smartfin-fw3/commit/c2deb321bf15a622c3ca0a74562833901dc2490e))

## [3.12.1](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.12.0...v3.12.1) (2025-05-14)


### Bug Fixes

* Adds NVRAM interface and docstrings ([447cdfb](https://github.com/UCSD-E4E/smartfin-fw3/commit/447cdfbf7c183b531a63efa150bc39a8d3038040))
* consolidate water sensor array length ([#186](https://github.com/UCSD-E4E/smartfin-fw3/issues/186)) ([90dc412](https://github.com/UCSD-E4E/smartfin-fw3/commit/90dc412a6b37b695e611a3d945cf929728947488))
* removed inconsistent and redundant array length ([69f2a69](https://github.com/UCSD-E4E/smartfin-fw3/commit/69f2a69edf8dc3961596a1d8e850be7417247388))
* water sensor parameter removed ([55a207e](https://github.com/UCSD-E4E/smartfin-fw3/commit/55a207e67cbb179f4c4e3444eaac718cb9afeff7))
* workflow testing ([d257f31](https://github.com/UCSD-E4E/smartfin-fw3/commit/d257f3185bd3305895d99309e0f73f6fc659a148))

# [3.12.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.11.0...v3.12.0) (2025-05-09)


### Bug Fixes

* not supported implementation for non particle ([9ff8757](https://github.com/UCSD-E4E/smartfin-fw3/commit/9ff8757dbe97c50bf24a23ec1d7329e2496ec59e))


### Features

* Adds format ([b5b314d](https://github.com/UCSD-E4E/smartfin-fw3/commit/b5b314d64f4e5632c01a4b16bf2ce5e7cfcbe479))

# [3.11.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.10.0...v3.11.0) (2025-05-09)


### Bug Fixes

* Adds debugging statements and particle process ([99cea1f](https://github.com/UCSD-E4E/smartfin-fw3/commit/99cea1fc1e9d519fe6741e94a0290f975520a314))
* Adds FLOG entries ([f5799c4](https://github.com/UCSD-E4E/smartfin-fw3/commit/f5799c468c0529d491b6ae88cbe8c1b9a013ba4d))
* Adjusts open fail behavior ([55e3f4f](https://github.com/UCSD-E4E/smartfin-fw3/commit/55e3f4fc556ceccc4cac37ba92561dc798ee0068))
* Fixes delay computation ([b9fea05](https://github.com/UCSD-E4E/smartfin-fw3/commit/b9fea05d03c395d155047eafcd5b088d767e46bb))
* Fixes deploy ([#228](https://github.com/UCSD-E4E/smartfin-fw3/issues/228)) ([3bc9543](https://github.com/UCSD-E4E/smartfin-fw3/commit/3bc95435416fb64437d44500f88001b924f251e8)), closes [#226](https://github.com/UCSD-E4E/smartfin-fw3/issues/226)
* Prohibit loop on failure ([6ace4b8](https://github.com/UCSD-E4E/smartfin-fw3/commit/6ace4b87e51c1c8330c8978ab9c770a0092ca2cd))


### Features

* Changes CLI exit to default to STATE_CHARGE ([1544ee9](https://github.com/UCSD-E4E/smartfin-fw3/commit/1544ee95130cf03a3cb29ebebe79f4ed21b9cd19))

# [3.10.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.9.2...v3.10.0) (2025-05-07)


### Bug Fixes

* Adds debugging for cellular ([ad8820b](https://github.com/UCSD-E4E/smartfin-fw3/commit/ad8820b8b868636d746e8870ed278effe7a48676))
* Adds state transition justifications ([b4e0259](https://github.com/UCSD-E4E/smartfin-fw3/commit/b4e0259eb0e255f4fe2dd4c852bde63d8ca9c802))
* Adds test for temperature/water encoding ([2d2987c](https://github.com/UCSD-E4E/smartfin-fw3/commit/2d2987ca1928bafb354fafd609702cddba6d567e))
* Allows fast deploy from charge ([5859775](https://github.com/UCSD-E4E/smartfin-fw3/commit/58597759b2114533c237193740c1e26c41f133db))
* Cleans up output ([88769f3](https://github.com/UCSD-E4E/smartfin-fw3/commit/88769f3000aca6f1304eedf18d08809748f970dc))
* Fixes scheduler ([a761577](https://github.com/UCSD-E4E/smartfin-fw3/commit/a761577b766eab92eb08f88f70c53c20390e5f67))
* Fixing scheduler ([c0c604a](https://github.com/UCSD-E4E/smartfin-fw3/commit/c0c604aaebbe7f242c3572bd7db0d78254eb9573))
* Updates notebook run ([73456a4](https://github.com/UCSD-E4E/smartfin-fw3/commit/73456a4f615b3de2b2cbf49502f9f797a6db5b7e))


### Features

* Adds Ens 1 to the schedule ([ad2abef](https://github.com/UCSD-E4E/smartfin-fw3/commit/ad2abefaa8f73f0ad99a9d3b7886f9282ff3a38d))
* Adds Ensemble01 implementation ([7c46d5e](https://github.com/UCSD-E4E/smartfin-fw3/commit/7c46d5ef91ead2d3e9063a7361d1d480c1d9fd1f))

## [3.9.2](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.9.1...v3.9.2) (2025-04-29)


### Bug Fixes

* Adds timing output ([2a1525a](https://github.com/UCSD-E4E/smartfin-fw3/commit/2a1525aceef69897c50d8f892b2b83ea46f766fd))
* Allows deploy while in charge ([ff35549](https://github.com/UCSD-E4E/smartfin-fw3/commit/ff35549940dfcc42d4522f2cd1bbb506991bc166))
* Fixes context window ([acc968c](https://github.com/UCSD-E4E/smartfin-fw3/commit/acc968c4897c21a1b2643878754af9b821e13818))
* Fixes inWater flag behavior ([87b5328](https://github.com/UCSD-E4E/smartfin-fw3/commit/87b5328495892b6bca1fc394501dbba59d17ec2d))
* Fixes monitorSensors water ([fc39bc7](https://github.com/UCSD-E4E/smartfin-fw3/commit/fc39bc760a20bcfb5e1c318e87f3cb5d7ea28357))
* Fixes schele ([367280d](https://github.com/UCSD-E4E/smartfin-fw3/commit/367280d0c48da4c0056aafbfe5d7077f722d1f19))
* Fixes sensor utilization and staging ([a88d7d7](https://github.com/UCSD-E4E/smartfin-fw3/commit/a88d7d7d135111c3036aef59ebad43017486bdf7))
* Removes extraneous state configuration ([5999b32](https://github.com/UCSD-E4E/smartfin-fw3/commit/5999b328e59186df4dad31a7489caa3c3de0dc23))

## [3.9.1](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.9.0...v3.9.1) (2025-04-25)


### Bug Fixes

* Adds decimal output ([dfd6371](https://github.com/UCSD-E4E/smartfin-fw3/commit/dfd63711c2d8a7d9bd66e6d29f4072fc7c8d63f6))
* Consolidates charger removed messages ([564fc0b](https://github.com/UCSD-E4E/smartfin-fw3/commit/564fc0be0dd376e9b7427c25d27d4c79652842ab))
* Fixes PC Hal pin defs ([a097b9e](https://github.com/UCSD-E4E/smartfin-fw3/commit/a097b9e51622173eb4cbff55052f10d7f9e1f279))
* Fixes state sequencing ([9376c11](https://github.com/UCSD-E4E/smartfin-fw3/commit/9376c117446272be13a347ec79b75ea2a4e3e7f1))
* Fixes water sensor and LED ([ce1fa85](https://github.com/UCSD-E4E/smartfin-fw3/commit/ce1fa859405e66d8d65d67ca2b6c1bb050cc8c1c))
* Fixes water sensor behavior ([#208](https://github.com/UCSD-E4E/smartfin-fw3/issues/208)) ([5dc4ffd](https://github.com/UCSD-E4E/smartfin-fw3/commit/5dc4ffd0dbf34207274ceb6bab2669bffe3e65df))
* Fixing state transitions ([70805c0](https://github.com/UCSD-E4E/smartfin-fw3/commit/70805c05079dac3bd5884a31b031348338624bfe))

# [3.9.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.8.2...v3.9.0) (2025-04-22)


### Bug Fixes

* Adds charger and water check inhibit ([cfefff9](https://github.com/UCSD-E4E/smartfin-fw3/commit/cfefff94eb551e58c8176634733b866bd85a2b7c))
* Adds checks for DMP init, fix AD0 ([f650f7c](https://github.com/UCSD-E4E/smartfin-fw3/commit/f650f7c415b77580c5428662bf70b1f408813490))
* Adds debugging and timer config ([3cba017](https://github.com/UCSD-E4E/smartfin-fw3/commit/3cba017c5bd32b199cb6a7603a5a5ba148605c84))
* Adds setupICM ([174cb06](https://github.com/UCSD-E4E/smartfin-fw3/commit/174cb06ff385a153970cc6fe2fada7321198ebff))
* Consolidated ensembles ([5c6fdeb](https://github.com/UCSD-E4E/smartfin-fw3/commit/5c6fdebf5cd7bce53b458d8e2e27b013c91b71b7))
* Consolidates buffers ([28a5d83](https://github.com/UCSD-E4E/smartfin-fw3/commit/28a5d83d2ea20f7cae80a89e7c66d8b9fd8ee684))
* Disables ensemble ([c0bbc63](https://github.com/UCSD-E4E/smartfin-fw3/commit/c0bbc638bb126115382d62383b82977c590b79ab))
* Fixes charger task ride behavior ([f40711c](https://github.com/UCSD-E4E/smartfin-fw3/commit/f40711c0c6e7b19fa0be3ff626f2baba6c063b56))
* Fixes dummy ([f7c576b](https://github.com/UCSD-E4E/smartfin-fw3/commit/f7c576b97c3feb0fea33518c13ed8c44342d4d08))
* Fixes monitorSensors ([379b19a](https://github.com/UCSD-E4E/smartfin-fw3/commit/379b19aab2ff320dbb0180a492621d8854f9462b))
* Fixes PC build ([02c2344](https://github.com/UCSD-E4E/smartfin-fw3/commit/02c23446520620283bb579fa947c8c7877544064))
* Fixes PC build and moves ensembleTypes to deploy ([07384dc](https://github.com/UCSD-E4E/smartfin-fw3/commit/07384dc4cf6b2edd346e18b302d0a7e22deefd49))
* Implements resetReasonData for pc_hal ([15b200f](https://github.com/UCSD-E4E/smartfin-fw3/commit/15b200fb66193497aabf687678e98409350f0eaf))


### Features

* Adds additional system dump command ([ffa2322](https://github.com/UCSD-E4E/smartfin-fw3/commit/ffa23221056bcf45edbff8c9d4355253167818f3))
* Adds menu for system dump ([d9ee06c](https://github.com/UCSD-E4E/smartfin-fw3/commit/d9ee06c12be5034255b2f68ad4d743847900eae3))
* Adds reset reason data logging ([14afb1f](https://github.com/UCSD-E4E/smartfin-fw3/commit/14afb1f2a4fe91130ecf3e8016cd5ad25c041367))
* Implementing ensembles and ride task ([75e7561](https://github.com/UCSD-E4E/smartfin-fw3/commit/75e7561d91d4e24694bf6621f6ba1bfc700cb3c1))

## [3.8.2](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.8.1...v3.8.2) (2025-04-14)


### Bug Fixes

* Adds Particle.process to loop ([127ede1](https://github.com/UCSD-E4E/smartfin-fw3/commit/127ede146ccdc2666be39c94c5f12f5273d0ff76))
* Adds Particle.process to loop ([#198](https://github.com/UCSD-E4E/smartfin-fw3/issues/198)) ([86ed434](https://github.com/UCSD-E4E/smartfin-fw3/commit/86ed434d362a55e3acb7df632d2cb172d130c7fe))

## [3.8.1](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.8.0...v3.8.1) (2025-04-14)


### Bug Fixes

* Consolidated DMP data access ([48bffc3](https://github.com/UCSD-E4E/smartfin-fw3/commit/48bffc398aa8f3c28af3759d7d863b0ded98cc68))

# [3.8.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.7.2...v3.8.0) (2025-04-14)


### Bug Fixes

* Adds error catching for monitorSensors ([a0c5e3a](https://github.com/UCSD-E4E/smartfin-fw3/commit/a0c5e3ac510f3a66d5f298f01b4925f3c51fa456))
* Adds platform guards ([27ca69b](https://github.com/UCSD-E4E/smartfin-fw3/commit/27ca69bc8e19562e0660da95c90342e04c5d98e4))


### Features

* Adds DMP functions ([fd2d848](https://github.com/UCSD-E4E/smartfin-fw3/commit/fd2d848628854add6c66b7ed7ee84b7eb2f42246))
* Adds monitorSensors ([d837a91](https://github.com/UCSD-E4E/smartfin-fw3/commit/d837a919dcb87f79a3db4827e02ca323343bd5f8))
* vendored ICM-20948, enables DMP ([4e04b69](https://github.com/UCSD-E4E/smartfin-fw3/commit/4e04b69215dce11a6377e9578f3cbdfe4a4921d2))

## [3.7.2](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.7.1...v3.7.2) (2025-04-10)


### Bug Fixes

* consolidated SleepTask::BootBehavior accesses to NVRAM accesses ([274fc24](https://github.com/UCSD-E4E/smartfin-fw3/commit/274fc2435dd786c4b55da6ab69f48803fd45bcae))
* consolidated SleepTask::BootBehavior accesses to NVRAM accesses ([#187](https://github.com/UCSD-E4E/smartfin-fw3/issues/187)) ([a368ae6](https://github.com/UCSD-E4E/smartfin-fw3/commit/a368ae61a42c31d241d6af6cf62e9ca24cb87445))

## [3.7.1](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.7.0...v3.7.1) (2025-03-30)


### Bug Fixes

* implement error logging for Temp read ([26098f0](https://github.com/UCSD-E4E/smartfin-fw3/commit/26098f011e377f02281d81b1ee01dbe2f4de9076))
* IMU get sensor readings return values closes [#130](https://github.com/UCSD-E4E/smartfin-fw3/issues/130) ([#165](https://github.com/UCSD-E4E/smartfin-fw3/issues/165)) ([3c9b845](https://github.com/UCSD-E4E/smartfin-fw3/commit/3c9b8450e8ec02388ad929c4793031fab597e355))

# [3.7.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.6.3...v3.7.0) (2025-03-02)


### Features

* data collection now averages data ([8fdff1a](https://github.com/UCSD-E4E/smartfin-fw3/commit/8fdff1ab17509547e105f0282d3fe89f6e3e8471))
* data collection now averages data ([#145](https://github.com/UCSD-E4E/smartfin-fw3/issues/145)) ([90761be](https://github.com/UCSD-E4E/smartfin-fw3/commit/90761bed4dd9465c1303e234d118f7918877518c))
* data collection now averages data properly with correct naming ([9b598bc](https://github.com/UCSD-E4E/smartfin-fw3/commit/9b598bc32343d743b49e2ceb09d3bc4863806c84))

## [3.6.3](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.6.2...v3.6.3) (2025-02-10)


### Bug Fixes

* corrected format specifier ([f670c73](https://github.com/UCSD-E4E/smartfin-fw3/commit/f670c73402ad733dfe3c0aa270f5f479d440bc6b))
* corrected format specifier ([#170](https://github.com/UCSD-E4E/smartfin-fw3/issues/170)) ([d074a97](https://github.com/UCSD-E4E/smartfin-fw3/commit/d074a97182b7b2c7299306b3c6c8f6f537f9f1ab))

## [3.6.2](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.6.1...v3.6.2) (2025-02-05)


### Bug Fixes

* in DataUpload, commented unused function & added docs ([c926f21](https://github.com/UCSD-E4E/smartfin-fw3/commit/c926f212c53b3e385c55a1b61574d8f6ab9332e3))
* in DataUpload, commented unused function & added docs  ([#132](https://github.com/UCSD-E4E/smartfin-fw3/issues/132)) ([777380a](https://github.com/UCSD-E4E/smartfin-fw3/commit/777380ad6021f252d7ebc6f2507c77d296e6480e))

## [3.6.1](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.6.0...v3.6.1) (2025-01-27)


### Bug Fixes

* changed format specifier ([fad43fc](https://github.com/UCSD-E4E/smartfin-fw3/commit/fad43fc5a41b1593af1dcb7816b7ffed2e94ab89))
* explicit cast variable to prevent warning ([d7ebf8b](https://github.com/UCSD-E4E/smartfin-fw3/commit/d7ebf8b04a521750cc5f02ba3485e3f2fd301f7e))
* explicit cast variable to prevent warning ([#157](https://github.com/UCSD-E4E/smartfin-fw3/issues/157)) ([9e4df01](https://github.com/UCSD-E4E/smartfin-fw3/commit/9e4df01d252f7e5db92cf9713c757f48247d7731))
* used <cinttypes> to fix formatting issue ([1791138](https://github.com/UCSD-E4E/smartfin-fw3/commit/17911380b91d4f625032d472da9326f039452760))

# [3.6.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.5.0...v3.6.0) (2025-01-07)


### Bug Fixes

* Abstracted particle specific methods ([913fa96](https://github.com/UCSD-E4E/smartfin-fw3/commit/913fa960cd8af21638512247013b8329015c935c))
* Adds __SPISettings ([d47eaec](https://github.com/UCSD-E4E/smartfin-fw3/commit/d47eaecd1641cff93bde2de249a46fbcbe9c6d63))
* Adds debugCommands.cpp ([9279017](https://github.com/UCSD-E4E/smartfin-fw3/commit/9279017136bde9c2314ca96eb0897a02391cb966))
* Adds dummies ([b718051](https://github.com/UCSD-E4E/smartfin-fw3/commit/b71805193620b5cc45ccb8bf6a729bc86d6c5555))
* Adds dummy classes ([8735504](https://github.com/UCSD-E4E/smartfin-fw3/commit/87355041fb5a91d479b20ede1fa0fa57a0478830))
* Adds dummy classes ([a5eb618](https://github.com/UCSD-E4E/smartfin-fw3/commit/a5eb618f2be710cc6207cd6acb0b33be595a980b))
* Adds dummy EEPROM ([1cdac32](https://github.com/UCSD-E4E/smartfin-fw3/commit/1cdac32884bbc1c75f98d47311c9f4ff13d409a8))
* Adds dummy modules ([2d099bc](https://github.com/UCSD-E4E/smartfin-fw3/commit/2d099bc513dc3241f7ac6d839bee71f028393acb))
* Adds dummy modules ([e923893](https://github.com/UCSD-E4E/smartfin-fw3/commit/e923893b2da656c710bd8b09fa2a41490a48b921))
* Adds dummy Serial ([4dd1d5e](https://github.com/UCSD-E4E/smartfin-fw3/commit/4dd1d5eaa0392b10b7ae1e129b18dc033c8c7a83))
* Adds fileCLI ([45a69ef](https://github.com/UCSD-E4E/smartfin-fw3/commit/45a69ef0a7266d2981fe65d9398022ea67436f55))
* Adds flog and conio ([5584ff9](https://github.com/UCSD-E4E/smartfin-fw3/commit/5584ff9eae36d9002c2ee334d463fbe7330a0171))
* Adds functional include ([c204edf](https://github.com/UCSD-E4E/smartfin-fw3/commit/c204edf1a2248ea9ca04d93a88d531bb2c0961ae))
* Adds GPS includes ([94e0e77](https://github.com/UCSD-E4E/smartfin-fw3/commit/94e0e77c0326fbac6dc039a14dc640f2f9bf9023))
* Adds LED ([db7c473](https://github.com/UCSD-E4E/smartfin-fw3/commit/db7c473c3156ed6b3dab78cf2aad10c44e0ac0ad))
* Adds location service getLocation ([50f0538](https://github.com/UCSD-E4E/smartfin-fw3/commit/50f0538b26185c83520393c8bd8de23861817379))
* Adds LocationService dummy ([4f1f1f8](https://github.com/UCSD-E4E/smartfin-fw3/commit/4f1f1f8a528f4171ea114c8cbe3501d1f1ffbfd7))
* Adds logger ([39ae403](https://github.com/UCSD-E4E/smartfin-fw3/commit/39ae40346b97528349feb747c6665cbd1616997d))
* Adds LogLevel dummy ([7b73d0d](https://github.com/UCSD-E4E/smartfin-fw3/commit/7b73d0dac9b65ddb4c746a66cf22e34ea2ab5b12))
* Adds Particle ([d70a187](https://github.com/UCSD-E4E/smartfin-fw3/commit/d70a1875d7d101d15e0637b7324032400a8d7949))
* Adds Serial Write ([d82b475](https://github.com/UCSD-E4E/smartfin-fw3/commit/d82b47534176c8d9041af3351b96c33556ebb5ba))
* Adds sigint handlers ([278314f](https://github.com/UCSD-E4E/smartfin-fw3/commit/278314f975b4d3b0adf7c51859187ec392ab76f9))
* Adds SPI for PC HAL ([0360073](https://github.com/UCSD-E4E/smartfin-fw3/commit/03600735820341d8c9fdf6b5926fc2a217fbe863))
* Adds SPIClass dummy ([1de15cc](https://github.com/UCSD-E4E/smartfin-fw3/commit/1de15cc05ecb32330a881c5d5f77b37bc442f091))
* Adds stdarg ([17489a5](https://github.com/UCSD-E4E/smartfin-fw3/commit/17489a589464b1912edb1d25115f655d144d5b09))
* Adds String dummy ([340a931](https://github.com/UCSD-E4E/smartfin-fw3/commit/340a9317601f3339a7275884b677bcf229c91fe0))
* Adds String methods ([531fd5c](https://github.com/UCSD-E4E/smartfin-fw3/commit/531fd5c2a7ab388132aadf9e759a4644c8108e77))
* Adds System dummy ([43b8e25](https://github.com/UCSD-E4E/smartfin-fw3/commit/43b8e25d827f4ca5b87a7ea16a3f7c1cdafed744))
* Adds system.cpp ([66670f1](https://github.com/UCSD-E4E/smartfin-fw3/commit/66670f1654910395324e2fdaec846242ab88781c))
* Adds Timer dummy class ([eacf510](https://github.com/UCSD-E4E/smartfin-fw3/commit/eacf51077b6bc4c73ee4fcbf44534a3f3f0cb465))
* Adds USARTSerial dummy class ([b35a691](https://github.com/UCSD-E4E/smartfin-fw3/commit/b35a6911b8d7150a222aa83dcbec966be3aec9c2))
* CRC and SHA256 only on Particle ([14be8ff](https://github.com/UCSD-E4E/smartfin-fw3/commit/14be8ff6a6a3b1823f2c5fcfffeee74b592ceb8e))
* Disables recorder ([ef08445](https://github.com/UCSD-E4E/smartfin-fw3/commit/ef08445f970a98214e7929c8a808fcc2e59911e0))
* Dummy I2C ([8712770](https://github.com/UCSD-E4E/smartfin-fw3/commit/871277080bbc1b430b3f9818ebd65afb7ddabda7))
* Fixes docs ([376e21f](https://github.com/UCSD-E4E/smartfin-fw3/commit/376e21f6ca331f25783b36d699b0a2608bb9ea15))
* Implemented dummy location service ([dcdf943](https://github.com/UCSD-E4E/smartfin-fw3/commit/dcdf943bfc986ae4302cabd981f6caebd5034239))
* Implements EEPROM ([0dad497](https://github.com/UCSD-E4E/smartfin-fw3/commit/0dad497046e55a4f721cd45fa5f555b753d3b42d))
* Implements LEDSystemTheme ([3ac0640](https://github.com/UCSD-E4E/smartfin-fw3/commit/3ac0640b70bcef3f5d80c0db7b8ff8eb18ff4c5e))
* Implements pin functions ([5041831](https://github.com/UCSD-E4E/smartfin-fw3/commit/5041831fa2ec5ed7624f06e44a119fecbcdd53d7))
* Implements waterSensor ([200d869](https://github.com/UCSD-E4E/smartfin-fw3/commit/200d8697a63b037c662ccf746bbc3003f111bcab))
* IMU dummy ([45d06d6](https://github.com/UCSD-E4E/smartfin-fw3/commit/45d06d6b1c4e233d0b671405ef6c488d1335e864))
* Removes unused function ([5b40a24](https://github.com/UCSD-E4E/smartfin-fw3/commit/5b40a249b2bb71b5d3ce93374856312bcad14ed3))
* renames getline to SF_OSAL_getline ([a372b9c](https://github.com/UCSD-E4E/smartfin-fw3/commit/a372b9c74ee23e1a2bcbe5559564e76892abc943))
* Replaces kbhit ([6745b45](https://github.com/UCSD-E4E/smartfin-fw3/commit/6745b45651aa112a6b32535e1bf68b28a5eaf9db))
* Replaces newline constant ([a95d92f](https://github.com/UCSD-E4E/smartfin-fw3/commit/a95d92f2855d13ee7072f18230c7fe4de670774c))
* systemCommands compile ([5819e4f](https://github.com/UCSD-E4E/smartfin-fw3/commit/5819e4f9c01ae797af310d17a13ca48eab16a99d))
* Updates deploy.cpp for PC HAL ([e13d7b3](https://github.com/UCSD-E4E/smartfin-fw3/commit/e13d7b3640d01daf41df5ef62256d382a4ef8ec5))
* Updates platform values ([d8397fd](https://github.com/UCSD-E4E/smartfin-fw3/commit/d8397fd46b0b5f25a745f9bc5ca24f0c5e2e16dc))
* util compile ([9769a34](https://github.com/UCSD-E4E/smartfin-fw3/commit/9769a349cef42b059726a82cac36fd043765b800))


### Features

* Adds basic conio ([782972c](https://github.com/UCSD-E4E/smartfin-fw3/commit/782972c6a16d28d2b96d007217641336aac5b541))
* Adds glibc implementation ([044ef73](https://github.com/UCSD-E4E/smartfin-fw3/commit/044ef73cd6e3ecbd4d3b22dd83ae36791c82d16f))
* Adds pc hal ([1e87b30](https://github.com/UCSD-E4E/smartfin-fw3/commit/1e87b30e3cf851a65994438e33df9bda88dae3ee))
* Adds printf ([b8b53ce](https://github.com/UCSD-E4E/smartfin-fw3/commit/b8b53ce9fd122da6455595b3c67084f5aaf87755))
* Adds utils random ([c4126e1](https://github.com/UCSD-E4E/smartfin-fw3/commit/c4126e12f48ec0292bc549dc325178aada7cdd10))
* Basic ncurses interface ([6c2b358](https://github.com/UCSD-E4E/smartfin-fw3/commit/6c2b358839f90893df81b3ab9d6112e445ae0241))
* Switches getch ([cb92f7a](https://github.com/UCSD-E4E/smartfin-fw3/commit/cb92f7a6851d10a39f3653f0fa37ab372479040c))

# [3.5.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.4.0...v3.5.0) (2025-01-07)


### Features

* switched backslashes to forward slashes ([c23d9d6](https://github.com/UCSD-E4E/smartfin-fw3/commit/c23d9d609e0f9bf91a3db291bc4dc5c9e97ca05c))
* switched backslashes to forward slashes ([#161](https://github.com/UCSD-E4E/smartfin-fw3/issues/161)) ([2ca2141](https://github.com/UCSD-E4E/smartfin-fw3/commit/2ca214110213d5e7c0850c2ef229233106f8f239))

# [3.4.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.3.3...v3.4.0) (2024-12-06)


### Features

* integrated water sensor into data collection ([19bee51](https://github.com/UCSD-E4E/smartfin-fw3/commit/19bee5190dec2b74050a940a6e61bb2def8d7ca2))
* integrated water sensor into data collection ([#146](https://github.com/UCSD-E4E/smartfin-fw3/issues/146)) ([5e3ad08](https://github.com/UCSD-E4E/smartfin-fw3/commit/5e3ad08988300a0d64b60a0f7ca46c31b25a7497))

## [3.3.3](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.3.2...v3.3.3) (2024-11-22)


### Bug Fixes

* Adds cstdarg include ([1ea0cc8](https://github.com/UCSD-E4E/smartfin-fw3/commit/1ea0cc8a4fb4795d1d6840bc629c41659805b8cc))
* Adds cstdarg include ([#124](https://github.com/UCSD-E4E/smartfin-fw3/issues/124)) ([ffd41d7](https://github.com/UCSD-E4E/smartfin-fw3/commit/ffd41d7ec059ed799ddd2ff8d225641ae0b47a5c))
* Switches to limits.h ([ee94566](https://github.com/UCSD-E4E/smartfin-fw3/commit/ee945662148bf821dabdfe3fc71311e718f389c7))
* Switches to limits.h ([#129](https://github.com/UCSD-E4E/smartfin-fw3/issues/129)) ([379bcae](https://github.com/UCSD-E4E/smartfin-fw3/commit/379bcae1a4cb84f044ee2d4f2685df5f4ec72817))

## [3.3.2](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.3.1...v3.3.2) (2024-11-12)


### Bug Fixes

* fields inside menu_ documented. revert unintended changes ([fd3ef27](https://github.com/UCSD-E4E/smartfin-fw3/commit/fd3ef27c16882a7c122ad6b22777aba013098a8a))

## [3.3.1](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.3.0...v3.3.1) (2024-11-07)


### Bug Fixes

* solve recorder_debug members not documented doxygen warning ([782545e](https://github.com/UCSD-E4E/smartfin-fw3/commit/782545eb9bfb87eef926e6378801a4620708c411))
* solve recorder_debug members not documented doxygen warning ([88403a5](https://github.com/UCSD-E4E/smartfin-fw3/commit/88403a58068477e6b5a22e93c513cb8d70398110))
* solve recorder_debug members not documented doxygen warning ([#112](https://github.com/UCSD-E4E/smartfin-fw3/issues/112)) ([055fb6c](https://github.com/UCSD-E4E/smartfin-fw3/commit/055fb6c9da905c2d56168fe80457d30d17b7e423))

# [3.3.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.2.3...v3.3.0) (2024-11-01)


### Features

* Adds platform selection macro ([711778b](https://github.com/UCSD-E4E/smartfin-fw3/commit/711778b5a731f98f495f1ffb8b9b8bf5a27da325))
* Adds platform selection macro ([#113](https://github.com/UCSD-E4E/smartfin-fw3/issues/113)) ([18adff5](https://github.com/UCSD-E4E/smartfin-fw3/commit/18adff584e4d9db1d60cb3c22ff568566689ad11))

## [3.2.3](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.2.2...v3.2.3) (2024-11-01)


### Bug Fixes

* Updates components ([57f59da](https://github.com/UCSD-E4E/smartfin-fw3/commit/57f59da218af8e4a61e9b8c8711fad65c7e0aa09))

## [3.2.2](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.2.1...v3.2.2) (2024-10-30)


### Bug Fixes

* add brief & details to chargetask documentation ([42ed2e6](https://github.com/UCSD-E4E/smartfin-fw3/commit/42ed2e678e276cd26609116464ea40163d388323))
* add brief & details to ChargeTask documentation ([#105](https://github.com/UCSD-E4E/smartfin-fw3/issues/105)) ([0c004d8](https://github.com/UCSD-E4E/smartfin-fw3/commit/0c004d8c552b7aadc827b1e97362f7a6ce7d9de0))

## [3.2.1](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.2.0...v3.2.1) (2024-06-23)


### Bug Fixes

* adds arrows, calibration ([1285620](https://github.com/UCSD-E4E/smartfin-fw3/commit/1285620108e6a3083b54d4c656df5841d32274c5))

# [3.2.0](https://github.com/UCSD-E4E/smartfin-fw3/compare/v3.1.0...v3.2.0) (2024-02-10)


### Bug Fixes

*  add newline to water readings ([191f30f](https://github.com/UCSD-E4E/smartfin-fw3/commit/191f30f32cb06d9625b2537c9693d33ff950d47a))
* add mfg test command to main cli menu ([89845c4](https://github.com/UCSD-E4E/smartfin-fw3/commit/89845c4bae7a2d926cf9cbc19aa72bcb0d27689e))
* Added check for system descriptor ([bcb8bab](https://github.com/UCSD-E4E/smartfin-fw3/commit/bcb8babfa10c529e63e6950035ecfd3e8e235f04))
* Added FLOG include ([24cafec](https://github.com/UCSD-E4E/smartfin-fw3/commit/24cafec47ed6728ba6dac1553501baf2fb375c2b))
* Adds action logic ([d36f67f](https://github.com/UCSD-E4E/smartfin-fw3/commit/d36f67f8854712fffe0997b3a77355ad2a1be925))
* Adds check for boot behaviors ([4ddcbc5](https://github.com/UCSD-E4E/smartfin-fw3/commit/4ddcbc560e6e2ab2a732b64f3e38d2d332c5bb45))
* Adds chgdir ([3422dcb](https://github.com/UCSD-E4E/smartfin-fw3/commit/3422dcb1ab080a21d22faa404e83b7a18db79d4f))
* Adds connect shortcut ([5e79c51](https://github.com/UCSD-E4E/smartfin-fw3/commit/5e79c51288be6e59ae2ebef22cf2074238395cb6))
* Adds debug point flog ([32349fe](https://github.com/UCSD-E4E/smartfin-fw3/commit/32349fe5aea51750079618519b1d48d89b30bc9e))
* Adds device os version ([50535dc](https://github.com/UCSD-E4E/smartfin-fw3/commit/50535dcf29d5288f5768ec907f783e60b61e102e))
* Adds function to convert boot behavior to strings ([8b09693](https://github.com/UCSD-E4E/smartfin-fw3/commit/8b0969387e0005dff75dea2065b0d7fd8f2204bc))
* Adds initial style guide ([f2fc5b6](https://github.com/UCSD-E4E/smartfin-fw3/commit/f2fc5b650f2731080914a5f881ff60b752df8e8e))
* Adds name table ([0ffdabf](https://github.com/UCSD-E4E/smartfin-fw3/commit/0ffdabfa8308582663bc98bd6faa9a2f458b2be2))
* Adds name to deploy open debug print ([4293b17](https://github.com/UCSD-E4E/smartfin-fw3/commit/4293b1797f4d12c661fd34a9640cdf7954bfb8df))
* Adds NVRAM display ([9e4628f](https://github.com/UCSD-E4E/smartfin-fw3/commit/9e4628f854a582bc49458f683a757f30005a42d3))
* Adds protection against double open ([63b10dd](https://github.com/UCSD-E4E/smartfin-fw3/commit/63b10dd54e6a6349bf69b97fbbafee31126ba2d3))
* Adds state change ([89cf69a](https://github.com/UCSD-E4E/smartfin-fw3/commit/89cf69aab3e53c9f8db0ee6e949c3dc96d9b4172))
* Cleaned up flog entries ([8720114](https://github.com/UCSD-E4E/smartfin-fw3/commit/87201149ffaf3291b108635387f2296974beab4f))
* Cleans up mfg test logic ([2d07d44](https://github.com/UCSD-E4E/smartfin-fw3/commit/2d07d4484c7c48b7466126b1e2ec4f5dc673c178))
* corrected power detect pin ([19e7991](https://github.com/UCSD-E4E/smartfin-fw3/commit/19e7991abc67ee9574063d4ff605624edb98ed6d))
* delete duplicated code ([85bc539](https://github.com/UCSD-E4E/smartfin-fw3/commit/85bc539b5f326e439bf20735399dbfe2605e9bf6))
* Enabled cloud connectivity commands ([7339458](https://github.com/UCSD-E4E/smartfin-fw3/commit/7339458e899396b1d852b06ace0f2ddd934ecd71))
* escape from chargestate when charger removed ([1082af7](https://github.com/UCSD-E4E/smartfin-fw3/commit/1082af71197e263be39c53c7b99f26cb7c0c68c5))
* fix errors after merging conflicts ([d0c65df](https://github.com/UCSD-E4E/smartfin-fw3/commit/d0c65df29024ef6e54cb4f4476869661425e2835))
* Fixes charger check ([82e6ea2](https://github.com/UCSD-E4E/smartfin-fw3/commit/82e6ea25ee963ebba8152b59efcc0155ad41ea2c))
* FIxes Check Charging output ([880b8a8](https://github.com/UCSD-E4E/smartfin-fw3/commit/880b8a885e67b8b6b2202a3ebfbe6abc104ba539))
* Fixes connection behavior ([949c8bb](https://github.com/UCSD-E4E/smartfin-fw3/commit/949c8bb647d33818efe8533806abaa6ed25962f0))
* Fixes current_dir level ([815c6a1](https://github.com/UCSD-E4E/smartfin-fw3/commit/815c6a1b0058437e852ce21384fb082e52985c2c))
* Fixes data location initialization ([4d6f328](https://github.com/UCSD-E4E/smartfin-fw3/commit/4d6f328212e417eb1128e3ebcf2934fc281fd509))
* Fixes deploy open debug print ([4bfd7bd](https://github.com/UCSD-E4E/smartfin-fw3/commit/4bfd7bdbe4fe29e730658263770877ea0327a331))
* Fixes display formats ([0f7c370](https://github.com/UCSD-E4E/smartfin-fw3/commit/0f7c3700e39c7a4e5b6d0a578577e667369429aa))
* Fixes fileCLI b85dump buffer sizes ([683d59b](https://github.com/UCSD-E4E/smartfin-fw3/commit/683d59b7f643aa73e43d876c225175d5de74688b))
* Fixes includes ([cb0da06](https://github.com/UCSD-E4E/smartfin-fw3/commit/cb0da060cf9f7e094dec601e7eebc7ffc5ed9b22))
* Fixes LED mapping and charge status ([80548f0](https://github.com/UCSD-E4E/smartfin-fw3/commit/80548f0fc96e26ca8a065ebc5304ba9c2ece7776))
* Fixes looping behavior and optimizes upload sizes ([55e4a96](https://github.com/UCSD-E4E/smartfin-fw3/commit/55e4a96249a23fc76ebd854525c09e9dd735a8c0))
* Fixes newlines ([563e2e3](https://github.com/UCSD-E4E/smartfin-fw3/commit/563e2e3d6b4d86da274124a13a0a3df8bdc424a2))
* fixes Recorder::getLastPacket return value ([a44a3df](https://github.com/UCSD-E4E/smartfin-fw3/commit/a44a3df8d48addacdc53aa7e8190aca74b82d084))
* Fixes sleep CLI ([56c243a](https://github.com/UCSD-E4E/smartfin-fw3/commit/56c243adc2b18a66c496bd71edef155dcc73d1d2))
* Fixes static prototypes and docstrings ([9710fb7](https://github.com/UCSD-E4E/smartfin-fw3/commit/9710fb7520c3754b8cc9baa583f98e0b6dc6e55e))
* Fixes tooling paths and switches to c++11 ([b74b46d](https://github.com/UCSD-E4E/smartfin-fw3/commit/b74b46da2594b692cc03409f5ebfc965f13e0621))
* Fixes upload CLI ([f23356f](https://github.com/UCSD-E4E/smartfin-fw3/commit/f23356f474c99711d9d5a52f451275c530463e39))
* Fixes USB power detect ([affac88](https://github.com/UCSD-E4E/smartfin-fw3/commit/affac884ec10745bbc81710d6e7642668c5459f6))
* Includes ([bd0ae1e](https://github.com/UCSD-E4E/smartfin-fw3/commit/bd0ae1e894207eb2cf8793cfd98f8b827a6d9a7a))
* mark static functions as private in header ([fadb871](https://github.com/UCSD-E4E/smartfin-fw3/commit/fadb871c989c6af8097d91d2993a6960cd18734f))
* Moves chargerTimer after static prototypes ([597040d](https://github.com/UCSD-E4E/smartfin-fw3/commit/597040d79d409c4f7e544e3887801a29bf07b792))
* Pops on empty session ([654abd5](https://github.com/UCSD-E4E/smartfin-fw3/commit/654abd548b46c576a2d9e065a89aa703222a5b78))
* Reduces flog entries to 128 ([6154c5b](https://github.com/UCSD-E4E/smartfin-fw3/commit/6154c5bad3cdd4baf9515a622df4f682581c537c))
* remove duplicates from merge ([c885e9b](https://github.com/UCSD-E4E/smartfin-fw3/commit/c885e9b1679a9d7ca5117cf1eef9bb525a0c01d5))
* remove unused defines in mfg test ([9ecf4d1](https://github.com/UCSD-E4E/smartfin-fw3/commit/9ecf4d17421782eb06b69e853d36aeb3844032ec))
* Removes setName ([e3cc6a5](https://github.com/UCSD-E4E/smartfin-fw3/commit/e3cc6a58a99969eda52e2c6a72fb85741e0b0d59))
* Removes SpiffsParticleRK dependency ([770cb2b](https://github.com/UCSD-E4E/smartfin-fw3/commit/770cb2b739f856981fd76a5559bbeca5de495d9a))
* Resolves CLI prompt ([d6ff364](https://github.com/UCSD-E4E/smartfin-fw3/commit/d6ff3648dc0dc00d7048d8a3247f125ee2d38e1f))
* return statement ([5b65ec9](https://github.com/UCSD-E4E/smartfin-fw3/commit/5b65ec91b3201f331c643cc83d55f3e2b352dc6a))
* Rolling back base85 ([825e778](https://github.com/UCSD-E4E/smartfin-fw3/commit/825e778dcb26359ddf6294fcd2ce71652427ad25))
* set correct wet/dry pins ([de3d2d1](https://github.com/UCSD-E4E/smartfin-fw3/commit/de3d2d1ab9f8d4ad9e91334e5fd0e56c7d0ba42a))
* stop the temp sensor when failed to start ([ac01093](https://github.com/UCSD-E4E/smartfin-fw3/commit/ac010934326912760e066d04a6d6acc305ef763e))
* stop using EIO ([ba13400](https://github.com/UCSD-E4E/smartfin-fw3/commit/ba13400015fe903b74156d57099ee937a40b8243))
* stop water timer when doing manufacturing test ([bcabc33](https://github.com/UCSD-E4E/smartfin-fw3/commit/bcabc330e949e517bef4f1c86f5cda42321fac61))
* swich icm driver to get magnetometor data ([f2646eb](https://github.com/UCSD-E4E/smartfin-fw3/commit/f2646ebcdb586892b02566e917ff6bfc416e3489))
* switch to statically allocated array for water detect ([c9b37d0](https://github.com/UCSD-E4E/smartfin-fw3/commit/c9b37d0b5f8c03f05ab4a398b739d3c6bd0709b0))
* Switches to sf::cloud API ([c8ab5ee](https://github.com/UCSD-E4E/smartfin-fw3/commit/c8ab5ee4247411a58c8fe2f72ed7e6500dc9e006))
* Switches to sf::cloud API ([22d800a](https://github.com/UCSD-E4E/smartfin-fw3/commit/22d800ab6f2e3b9a9e26a07ff85df6bc94a36931))
* update water detect pins ([15d07f3](https://github.com/UCSD-E4E/smartfin-fw3/commit/15d07f3eae23176a2cf1979a51080077c68925cb))
* Updates base85 to latest ([c41fe78](https://github.com/UCSD-E4E/smartfin-fw3/commit/c41fe780b51c11f3246b3939c3427c2f5d9cca1a))
* Updates flog ([86a4b45](https://github.com/UCSD-E4E/smartfin-fw3/commit/86a4b459b61876af58c7c3480579964b936abd13))
* Updates helptext ([be37dac](https://github.com/UCSD-E4E/smartfin-fw3/commit/be37dac36540e76d76e638fe978ee334e69f1b59))
* Updates hexdump ([47a6bd8](https://github.com/UCSD-E4E/smartfin-fw3/commit/47a6bd863d25066167dfd326237bf639b203ef80))
* Updates printState logic ([8f37227](https://github.com/UCSD-E4E/smartfin-fw3/commit/8f3722779a5e83cf9375f4d29a5a24249844f003))
* Updates Recorder::hasData return type ([d4d6981](https://github.com/UCSD-E4E/smartfin-fw3/commit/d4d69814b983f829089ebf577da83d052b650bc7))
* Updates WKP_PIN ([a4766b2](https://github.com/UCSD-E4E/smartfin-fw3/commit/a4766b204e4cb0d3e2378a6bc47b940402e2a6be))
* use system.batteryState for battery state instead of pin read ([89417a3](https://github.com/UCSD-E4E/smartfin-fw3/commit/89417a3918632b4d4fb1a57c4cbe279f41bdbed5))


### Features

* Added file publish dump ([263381a](https://github.com/UCSD-E4E/smartfin-fw3/commit/263381aa2d34e4567b3a7189940b70b91016bf1b))
* Adds additional fs flog entries ([98de874](https://github.com/UCSD-E4E/smartfin-fw3/commit/98de874a0bde74d5bd3dadd13ebccc958c1c91ec))
* Adds basic recorder functionality and test ([a9eb5e9](https://github.com/UCSD-E4E/smartfin-fw3/commit/a9eb5e993690dfca025bfa4fb4cda870ea226fed))
* Adds cd and pwd ([d35ff05](https://github.com/UCSD-E4E/smartfin-fw3/commit/d35ff055cee6ea3be342646d915970709c4912c0))
* Adds DeviceOS version display ([674701e](https://github.com/UCSD-E4E/smartfin-fw3/commit/674701eb5ffa7a3c164781a03acca843874a6d9f))
* Adds error handling to creation ([dc66939](https://github.com/UCSD-E4E/smartfin-fw3/commit/dc66939da122bf7cfb414ced6ea9136c0940e5b6))
* Adds firmware hashes ([2adc537](https://github.com/UCSD-E4E/smartfin-fw3/commit/2adc5378f619a2cd174b5c5536c12e2fd3f148b6))
* Adds fsync to write ([cf7b70e](https://github.com/UCSD-E4E/smartfin-fw3/commit/cf7b70e4ff726ccb1f65252b21a273d2de2b0a37))
* Adds helptext ([c2cfcd5](https://github.com/UCSD-E4E/smartfin-fw3/commit/c2cfcd5c49659707a8d00e26fc4099f49878d58a))
* Adds hexdump ([6344465](https://github.com/UCSD-E4E/smartfin-fw3/commit/63444655327eff4d0a5b8ed866f337501e5a9cca))
* Adds ls functionality ([66475c7](https://github.com/UCSD-E4E/smartfin-fw3/commit/66475c70bb67ee8ceaa576c0ea6413c974edad40))
* Adds mkdir ([f641578](https://github.com/UCSD-E4E/smartfin-fw3/commit/f641578a5d0f4f953a3ae5cf6d30d1049a696ce8))
* Adds packetizing ([6a574c0](https://github.com/UCSD-E4E/smartfin-fw3/commit/6a574c039283addbd3190162e92b2cc1b215093c))
* Adds pop last packet debug ([a296ff0](https://github.com/UCSD-E4E/smartfin-fw3/commit/a296ff0762ba66fceeb72d9c0719fca5634475e7))
* Adds publish_blob ([d2b7868](https://github.com/UCSD-E4E/smartfin-fw3/commit/d2b786842e4e5acf96243fca2ce662d6c6083306))
* Adds rate limiting ([72cda53](https://github.com/UCSD-E4E/smartfin-fw3/commit/72cda53f377b5fbd08f75b18684a2f15d8baad16))
* Adds reset reason logging ([177e05e](https://github.com/UCSD-E4E/smartfin-fw3/commit/177e05ef21bcb2bb79e58ecd3728caf99d7828b8))
* Adds reset reason tool ([209a59a](https://github.com/UCSD-E4E/smartfin-fw3/commit/209a59ac775caf3644e26c78ff0a6c0bde384c95))
* Adds rm, fixes buildpath ([08ad4c4](https://github.com/UCSD-E4E/smartfin-fw3/commit/08ad4c4db06946322060a5b1822618ad4af73d28))
* Adds Semantic Release ([71f75d7](https://github.com/UCSD-E4E/smartfin-fw3/commit/71f75d793502e053487173fc1dee29e94e478844)), closes [#38](https://github.com/UCSD-E4E/smartfin-fw3/issues/38)
* Adds setSessionTime ([1cdb02b](https://github.com/UCSD-E4E/smartfin-fw3/commit/1cdb02b45fd2afcc98442e4991f08b0c2627c27a))
* Adds sf::cloud::is_connected ([089185a](https://github.com/UCSD-E4E/smartfin-fw3/commit/089185a50e40ac0c614e7bac5dc07a34ab18ef97))
* Adds system battery descriptor ([62fe6c0](https://github.com/UCSD-E4E/smartfin-fw3/commit/62fe6c02e75ec8419e2f21da769b352a57991c91))
* Changes Recorder::putBytes return values ([2f2460e](https://github.com/UCSD-E4E/smartfin-fw3/commit/2f2460e00ea4192f15e69758688f32d6587257be))
* Completes recorder ([ab59ef8](https://github.com/UCSD-E4E/smartfin-fw3/commit/ab59ef84957fa9b7d5cda1628ed277791ea3e918))
* Functional session ([cd1effd](https://github.com/UCSD-E4E/smartfin-fw3/commit/cd1effd9baaf7276b35814ffbe00fc44bc59ad40))
* Merge pull request [#17](https://github.com/UCSD-E4E/smartfin-fw3/issues/17) from UCSD-E4E/chargetask ([2f845c0](https://github.com/UCSD-E4E/smartfin-fw3/commit/2f845c0b7aa659960091b446dee3787739fcae13))
* Switches to consolidated menu system ([1f58ab3](https://github.com/UCSD-E4E/smartfin-fw3/commit/1f58ab3ed5d0b46bd11aec32e4c7f7ccc1f1f44a))
* Updates wet/dry monitor output ([7108c2f](https://github.com/UCSD-E4E/smartfin-fw3/commit/7108c2f93ffc7bae6b25a5d2507603ae40790441))
