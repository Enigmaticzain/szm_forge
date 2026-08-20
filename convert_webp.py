from PIL import Image
import os
import subprocess

webp_path = "/home/szm7226/.gemini/antigravity/brain/c51a3cca-8649-4bd0-9d60-919d9fded77f/szm_forge_hackathon_demo_1786799682532.webp"
frames_dir = "webp_frames"
os.makedirs(frames_dir, exist_ok=True)

img = Image.open(webp_path)

# Extract all frames
frame_count = 0
try:
    while True:
        img.seek(frame_count)
        # Convert to RGB if it's RGBA
        frame = img.convert("RGB")
        frame.save(os.path.join(frames_dir, f"frame_{frame_count:05d}.jpg"))
        frame_count += 1
except EOFError:
    pass

print(f"Extracted {frame_count} frames.")

audio_duration = 87.48
if frame_count > 0:
    framerate = frame_count / audio_duration
    print(f"Target framerate: {framerate:.2f} fps")
    
    # Run ffmpeg to build the mp4
    subprocess.run([
        "ffmpeg", "-y", "-framerate", str(framerate), 
        "-i", f"{frames_dir}/frame_%05d.jpg", 
        "-c:v", "libx264", "-pix_fmt", "yuv420p", "video.mp4"
    ])
    
    # Combine with audio
    subprocess.run([
        "ffmpeg", "-y", "-i", "video.mp4", "-i", "voiceover_full.mp3",
        "-c:v", "copy", "-c:a", "aac", "-shortest",
        "/home/szm7226/.gemini/antigravity/brain/c51a3cca-8649-4bd0-9d60-919d9fded77f/szm_forge_hackathon_demo.mp4"
    ])
    print("Done!")
