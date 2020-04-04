## Software Requirements Specifications for Spring 2020 "Intro to Software Engineering" "Craft Topography" Project at Wright State Univesity
### Functional Requirements
  #### User Story A: As a Minecraft Player, I want a realistic terrain generation, so that i can have a more immersive experience.
   - Req 1.0 Craft implementation shall create a terrain from a topographic image
   - Req 1.1 Topographic image shall be a .png image
   - Req 1.2 Pixel values shall be normalized to represent a realistic range of block heights
   - Req 1.3 The realistic range of block heights shall be a minimum of 20 blocks and a maximum of 100 blocks
   - Req 3.0 One pixel in topographic image shall correspond to a 2x2 block area
  #### User Story B: As a Minecraft Player, I want block type variation based on different elevations in game so that i can have more realistic experience.
   - Req 2.0 At least three block types shall be generated in game. 
   - Req 2.1 Generated terrain blocks over 80 blocks high shall be snow blocks
   - Req 2.2 Generated block types shall vary based on block height
   - Req 4.0 The minimum terrain generated block height shall be 20 blocks
   - Req 4.1 The maximum terrain generated block height shall be 100 blocks
