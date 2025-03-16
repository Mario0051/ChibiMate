from PIL import Image, ImageDraw, ImageFont
import numpy as np

# Configuration
fps = 30  # frames per second
duration = 1  # duration in seconds
frames = fps * duration

# Load the image
try:
    base_img = Image.open('springfield.jpg')
except FileNotFoundError:
    print("Error: springfield.jpg not found in the current directory")
    exit(1)

# Convert to RGBA if it isn't already
if base_img.mode != 'RGBA':
    base_img = base_img.convert('RGBA')

# Get base image dimensions
width, height = base_img.width, base_img.height

# Define margins and target width
margin = 25
target_width = width - (2 * margin)

# Function to find the appropriate font size
def get_font_size_for_width(text, start_size=80):
    size = start_size
    try:
        font = ImageFont.truetype("Arial Bold.ttf", size)
    except:
        try:
            font = ImageFont.truetype("Arial.ttf", size)
        except:
            font = ImageFont.load_default()
            print("Warning: Arial font not found, using default font")
            return font, size

    # Create a dummy image for testing
    dummy_img = Image.new('RGBA', (width, height))
    dummy_draw = ImageDraw.Draw(dummy_img)
    
    # Binary search for the right font size
    min_size = 1
    max_size = 500  # reasonable upper limit
    
    while min_size <= max_size:
        mid_size = (min_size + max_size) // 2
        font = ImageFont.truetype("Arial Bold.ttf", mid_size)
        bbox = dummy_draw.textbbox((0, 0), text, font=font)
        current_width = bbox[2] - bbox[0]
        
        if abs(current_width - target_width) < 10:  # tolerance of 10 pixels
            return font, mid_size
        elif current_width > target_width:
            max_size = mid_size - 1
        else:
            min_size = mid_size + 1
    
    # If we exit the loop, use the last valid size
    font = ImageFont.truetype("Arial Bold.ttf", max_size)
    return font, max_size

text = "MOMMY"
font, font_size = get_font_size_for_width(text)

# Calculate text position
dummy_draw = ImageDraw.Draw(base_img)
text_bbox = dummy_draw.textbbox((0, 0), text, font=font)
text_width = text_bbox[2] - text_bbox[0]
text_height = text_bbox[3] - text_bbox[1]
text_x = (width - text_width) // 2  # Center horizontally
text_y = height - text_height - 20  # 20 pixels padding from bottom

frames_list = []

# Create a mask for the base image (full opacity)
mask = Image.new('L', (width, height), 255)

# Create frames
for frame in range(frames):
    # Calculate opacity (from 255 to 0)
    opacity = int(255 * (1 - frame/frames))
    
    # Create frame with fading image
    current_frame = Image.new('RGBA', (width, height), (255, 255, 255, 255))
    
    # Create fading mask
    current_mask = mask.point(lambda x: opacity)
    
    # Apply fading mask to base image
    faded_base = base_img.copy()
    faded_base.putalpha(current_mask)
    
    # Composite base image
    current_frame = Image.alpha_composite(current_frame, faded_base)
    
    # Create text layer with current opacity
    txt_layer = Image.new('RGBA', (width, height), (0, 0, 0, 0))
    txt_draw = ImageDraw.Draw(txt_layer)
    txt_draw.text((text_x, text_y), text, font=font, fill=(0, 0, 0, opacity))
    
    # Composite text layer
    current_frame = Image.alpha_composite(current_frame, txt_layer)
    
    frames_list.append(current_frame)

# Save as GIF
frames_list[0].save(
    'fading_text.gif',
    save_all=True,
    append_images=frames_list[1:],
    duration=1000//fps,  # Convert fps to milliseconds per frame
    loop=0
) 