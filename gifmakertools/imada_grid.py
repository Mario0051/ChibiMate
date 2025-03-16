from PIL import Image, ImageDraw, ImageFont
import numpy as np

# Create a new image with white background
width = 800
height = 1000
background_color = (255, 255, 255)

# Calculate grid parameters
rows = 5
cols = 4
cell_width = width // cols
cell_height = height // rows

def create_frame(visible_positions):
    # Create a new frame
    frame = Image.new('RGB', (width, height), background_color)
    draw = ImageDraw.Draw(frame)
    
    # Try to use a system font, or fall back to default
    try:
        font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 40)
    except:
        font = ImageFont.load_default()

    # Draw visible IMADA text
    for pos in visible_positions:
        row = pos // cols
        col = pos % cols
        x = col * cell_width + cell_width // 2
        y = row * cell_height + cell_height // 2
        text = "MAYA"
        # Get text size for centering
        bbox = draw.textbbox((x, y), text, font=font)
        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]
        # Draw centered text
        draw.text((x - text_width//2, y - text_height//2), text, fill=(0, 0, 0), font=font)
    
    return frame

# Create frames
frames = []
positions = list(range(20))  # 20 positions in total
for i in range(len(positions) + 1):
    frame = create_frame(positions[:i])
    frames.append(frame)

# Save as animated GIF
frames[0].save(
    'maya.gif',
    save_all=True,
    append_images=frames[1:],
    duration=300,  # Duration for each frame in milliseconds
    loop=0  # Loop forever
) 