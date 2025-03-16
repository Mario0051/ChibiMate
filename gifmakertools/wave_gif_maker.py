from PIL import Image
import numpy as np

# Configuration
width = 30000
height = 30000
fps = 30
duration = 4  # seconds for one complete cycle
frames = fps * duration

# Define color sequence (R, G, B)
color_sequence = [
    (255, 0, 0),      # Red
    (255, 255, 0),    # Yellow
    (255, 255, 255),  # White
    (0, 255, 255),    # Cyan
    (0, 0, 255),      # Blue
    (0, 0, 0),        # Black
    (0, 255, 0),      # Green
    (255, 255, 0),    # Yellow
    (255, 0, 0)       # Back to Red
]

def interpolate_colors(color1, color2, steps):
    """Interpolate between two colors."""
    return [
        tuple(int(c1 + (c2 - c1) * i / steps) 
              for c1, c2 in zip(color1, color2))
        for i in range(steps)
    ]

# Calculate frames per transition
frames_per_transition = frames // (len(color_sequence) - 1)

# Generate all frames' colors
all_colors = []
for i in range(len(color_sequence) - 1):
    transition_colors = interpolate_colors(
        color_sequence[i],
        color_sequence[i + 1],
        frames_per_transition
    )
    all_colors.extend(transition_colors)

# Ensure we have exactly the number of frames we want
while len(all_colors) < frames:
    all_colors.append(all_colors[-1])

# Create frames
frames_list = []
for i, color in enumerate(all_colors):
    # Create frame with solid color
    frame_array = np.full((height, width, 3), color, dtype=np.uint8)
    frame = Image.fromarray(frame_array)
    frames_list.append(frame)
    print(f"Frame {i+1}/{frames} complete")

print("Saving GIF...")
# Save as GIF
frames_list[0].save(
    'color_wave2.gif',
    save_all=True,
    append_images=frames_list[1:],
    duration=1000//fps,  # milliseconds per frame
    loop=0,
    optimize=False  # Don't optimize to maintain color quality
)
print("Done!") 