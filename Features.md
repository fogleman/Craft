## Minecraft Topography
### Feature 1
  #### Story A - Terrain generation from Topographic Images
   - Process topographic images to create an array of pixel values corresponding to block height.
   - Reformat pixel array to create bigger world.
      - e.g. image might be 1000x1000px, this would be a small area comparatively to the world, fix this by making each pixel correspond to a 2x2, 3x3, 4x4, etc... area of blocks.
   - Figure out map generation logic by creating completely flat world.
   - Implement map generation from pixel array.
   - Tweak map generation to create a more realistic world.
      - e.g. lowest block is 20 blocks from bottom of world.
      - e.g. blocks over 70 high are snowy.
  #### Story B - In-game Weather
   - Implement rain shaders.
   - Implement rain effect
      - Rain falling from the sky.
   - Implement darker clouds shaders.
   - Implement cloudiness.
      - More clouds, darker clouds and less light.
  
