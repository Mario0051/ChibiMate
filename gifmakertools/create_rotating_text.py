from PIL import Image, ImageDraw, ImageFont
import math

# Configuration
width = 500
height = 500
frames = 30  # Number of frames for one complete rotation
text = "IMADA"
background_color = (26, 26, 26)  # Dark background (same as HTML version)
text_color = (255, 255, 255)  # White text
radius = 150  # Radius of rotation
font_size = 36

# Create frames
frames_list = []

# Try to load a system font, fallback to default if not found
try:
    font = ImageFont.truetype("Arial.ttf", font_size)
except:
    font = ImageFont.load_default()

# Create each frame
for frame in range(frames):
    # Create new image with background
    image = Image.new('RGB', (width, height), background_color)
    draw = ImageDraw.Draw(image)
    
    # Calculate base rotation for this frame
    base_rotation = frame * (360 / frames)
    
    # Draw 8 text elements
    for i in range(8):
        # Calculate angle for each text
        angle = base_rotation + (i * 45)  # 45 degrees = 360/8
        
        # Convert angle to radians
        rad = math.radians(angle)
        
        # Calculate position
        center_x = width // 2
        center_y = height // 2
        x = center_x + radius * math.cos(rad)
        y = center_y + radius * math.sin(rad)
        
        # Calculate text rotation (90 degrees offset to make text tangent to circle)
        text_angle = angle + 90
        
        # Create rotated text
        txt_img = Image.new('RGBA', (200, 100), (0, 0, 0, 0))
        txt_draw = ImageDraw.Draw(txt_img)
        txt_draw.text((0, 0), text, font=font, fill=text_color)
        
        # Rotate text
        txt_img = txt_img.rotate(-text_angle, expand=True)
        
        # Calculate position to paste the rotated text
        paste_x = int(x - txt_img.width // 2)
        paste_y = int(y - txt_img.height // 2)
        
        # Paste the text onto main image
        image.paste(txt_img, (paste_x, paste_y), txt_img)
    
    frames_list.append(image)

# Save as GIF
frames_list[0].save(
    'rotating_imada.gif',
    save_all=True,
    append_images=frames_list[1:],
    duration=50,  # 50ms per frame = 20fps
    loop=0  # Loop forever
) 