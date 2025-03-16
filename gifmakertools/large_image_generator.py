import numpy as np
from PIL import Image
import time

def create_large_image(width=9625, height=9625):
    print(f"Starting image creation: {width}x{height}")
    start_time = time.time()
    
    # Create array in uint8 format to save memory (0-255 values)
    # Using numpy's efficient memory handling
    print("Allocating memory for image array...")
    image_array = np.zeros((height, width, 3), dtype=np.uint8)
    
    print("Generating pattern...")
    # Create a gradient pattern
    # Using vectorized operations for better performance
    x = np.linspace(0, 255, width)
    y = np.linspace(0, 255, height)
    X, Y = np.meshgrid(x, y)
    
    # Create RGB channels with different patterns
    image_array[:,:,0] = X % 256  # Red channel
    image_array[:,:,1] = Y % 256  # Green channel
    image_array[:,:,2] = ((X + Y) / 2) % 256  # Blue channel
    
    print("Converting to PIL Image...")
    # Convert to PIL Image
    image = Image.fromarray(image_array)
    
    print("Saving image...")
    # Save as PNG for lossless quality
    image.save("large_image.png", optimize=True)
    
    end_time = time.time()
    print(f"Image creation completed in {end_time - start_time:.2f} seconds")
    print(f"Image saved as 'large_image.png'")

if __name__ == "__main__":
    # Create the image
    create_large_image() 