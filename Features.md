## Minecraft Topography
### Feature 1
  #### Story A - Terrain generation from Topographic Images
   - Task 1: Process topographic images to create an array of pixel values.
   - Task 2: Normalize pixel values to give a valid range of numbers (see user story B for valid range)
   - Task 3: Reprocess pixel array in preparation to create bigger world.
      - e.g. image might be 1000x1000px, this would be a small area comparatively to the world, fix this by making each pixel correspond to a 2x2, 3x3, 4x4, etc... area of blocks.
   -  Task 4: Figure out map generation logic by creating completely flat world, then create a world that is x amount of blocks high
   - Task 5: Ensure each topographic image will be a .png file
   - Task 6:Implement map generation from pixel array.
  #### Story B - Block variation based on height
   - Task 1: Minimum block height will be 20 blocks and maximum block height will be 100 blocks
   - Task 2: Any blocks over 80 blocks will be snow blocks. 
   
