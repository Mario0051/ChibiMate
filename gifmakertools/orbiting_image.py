from PIL import Image, ImageDraw, ImageFont
import math

# Configuration
fps = 30
duration = 2  # seconds for one complete orbit
frames = fps * duration
orbit_radius = 3000  # increased to accommodate larger image

# Load the orbiting image
try:
    orbit_img = Image.open('belu.png')
except FileNotFoundError:
    print("Error: belu.png not found in the current directory")
    exit(1)

# Convert to RGBA if needed
if orbit_img.mode != 'RGBA':
    orbit_img = orbit_img.convert('RGBA')

# Resize orbiting image while maintaining aspect ratio
target_size = 2000
original_width, original_height = orbit_img.size
aspect_ratio = original_width / original_height

if aspect_ratio > 1:
    new_width = target_size
    new_height = int(target_size / aspect_ratio)
else:
    new_height = target_size
    new_width = int(target_size * aspect_ratio)

orbit_img = orbit_img.resize((new_width, new_height), Image.Resampling.LANCZOS)
orbit_width, orbit_height = orbit_img.size

# Create text
text = "PROFESSIONAL\nSTUPID"
min_height = 1000  # increased text height to match scale

# Function to find font size that makes text at least min_height tall
def get_font_size_for_height(text, min_height):
    size = 10  # start small
    try:
        font = ImageFont.truetype("Times New Roman.ttf", size)
    except:
        print("Times New Roman font not found")
        try:
            font = ImageFont.truetype("Times.ttf", size)
        except:
            print("Error: Times font not found")
            return ImageFont.load_default()

    # Binary search for the right font size
    min_size = 10
    max_size = 2000  # increased max size for larger text
    
    while min_size <= max_size:
        mid_size = (min_size + max_size) // 2
        font = ImageFont.truetype("Times New Roman.ttf", mid_size)
        bbox = ImageDraw.Draw(Image.new('RGBA', (1, 1))).textbbox((0, 0), text, font=font)
        current_height = bbox[3] - bbox[1]
        
        if current_height >= min_height:
            return font
        else:
            min_size = mid_size + 1
    
    return ImageFont.truetype("Times New Roman.ttf", min_size)

# Get font and calculate text size
font = get_font_size_for_height(text, min_height)
dummy = Image.new('RGBA', (1, 1))
bbox = ImageDraw.Draw(dummy).textbbox((0, 0), text, font=font)
text_width = bbox[2] - bbox[0]
text_height = bbox[3] - bbox[1]

# Create canvas large enough for text and orbit
canvas_width = max(text_width + orbit_width + orbit_radius * 2, 8000)
canvas_height = max(text_height + orbit_height + orbit_radius * 2, 8000)

# Calculate text position (centered)
text_x = (canvas_width - text_width) // 2
text_y = (canvas_height - text_height) // 2

# Create base frame with text (will be reused)
base_frame = Image.new('RGBA', (int(canvas_width), int(canvas_height)), (255, 255, 255, 255))
draw = ImageDraw.Draw(base_frame)
draw.text((text_x, text_y), text, font=font, fill=(0, 0, 0))

# Pre-calculate orbit positions
orbit_positions = []
orbit_center_x = canvas_width // 2
orbit_center_y = canvas_height // 2

for frame in range(frames):
    angle = 2 * math.pi * frame / frames
    x = orbit_center_x + math.cos(angle) * orbit_radius - orbit_width // 2
    y = orbit_center_y + math.sin(angle) * orbit_radius - orbit_height // 2
    orbit_positions.append((int(x), int(y)))

# Create frames
frames_list = []
for pos in orbit_positions:
    # Create new frame from base
    current_frame = base_frame.copy()
    
    # Paste orbiting image
    current_frame.paste(orbit_img, pos, orbit_img)
    
    # Resize frame to a more manageable size for GIF
    resized_frame = current_frame.resize((2000, 2000), Image.Resampling.LANCZOS)
    
    frames_list.append(resized_frame)
    print(f"Frame {len(frames_list)}/{frames} complete")

print("Saving GIF...")
# Save as GIF with optimized settings
frames_list[0].save(
    'orbiting_text.gif',
    save_all=True,
    append_images=frames_list[1:],
    duration=1000//fps,
    loop=0,
    optimize=True,
    quality=50  # Lower quality for faster saving
)
print("Done!") 