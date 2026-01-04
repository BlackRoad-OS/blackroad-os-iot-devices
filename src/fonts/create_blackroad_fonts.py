#!/usr/bin/env python3
"""
BlackRoad Mono Font Generator
Converts JetBrains Mono to ESP32-compatible VLWF format
"""

from PIL import Image, ImageDraw, ImageFont
import struct
import os

def create_vlw_font(ttf_path, output_path, size, name="BlackRoad Mono"):
    """Create a VLWF font file from TTF for ESP32"""
    
    print(f"Creating {name} {size}pt font...")
    
    try:
        font = ImageFont.truetype(ttf_path, size)
    except:
        print(f"Error loading font: {ttf_path}")
        return False
    
    # Character set - ASCII printable + common symbols
    chars = ''.join(chr(i) for i in range(32, 127))
    
    # Create font data
    glyph_data = []
    glyph_metrics = []
    
    for char in chars:
        # Create image for character
        bbox = font.getbbox(char)
        if bbox[2] == 0 or bbox[3] == 0:
            w, h = size, size
        else:
            w = bbox[2] - bbox[0]
            h = bbox[3] - bbox[1]
        
        # Create bitmap
        img = Image.new('L', (w, h), 0)
        draw = ImageDraw.Draw(img)
        draw.text((-bbox[0], -bbox[1]), char, font=font, fill=255)
        
        # Convert to 1-bit bitmap
        bitmap = []
        for y in range(h):
            for x in range(w):
                pixel = img.getpixel((x, y))
                bitmap.append(1 if pixel > 127 else 0)
        
        glyph_data.append(bitmap)
        glyph_metrics.append((w, h, bbox[0], bbox[1]))
    
    # Write VLWF header (simplified)
    with open(output_path, 'wb') as f:
        # Magic number + version
        f.write(b'\x23\x46\x4E\x54')  # "#FNT"
        f.write(struct.pack('<H', 2))  # Version 2
        
        # Font metrics
        f.write(struct.pack('<H', len(chars)))  # Number of glyphs
        f.write(struct.pack('<H', size))  # Font size
        
        # Write glyph data
        for i, char in enumerate(chars):
            w, h, x_off, y_off = glyph_metrics[i]
            bitmap = glyph_data[i]
            
            # Glyph header
            f.write(struct.pack('<H', ord(char)))  # Character code
            f.write(struct.pack('<H', w))  # Width
            f.write(struct.pack('<H', h))  # Height
            f.write(struct.pack('<h', x_off))  # X offset
            f.write(struct.pack('<h', y_off))  # Y offset
            
            # Bitmap data (1-bit packed)
            bytes_per_row = (w + 7) // 8
            for y in range(h):
                row_data = 0
                for x in range(w):
                    if bitmap[y * w + x]:
                        row_data |= (1 << (7 - (x % 8)))
                    if (x % 8 == 7) or (x == w - 1):
                        f.write(struct.pack('B', row_data))
                        row_data = 0
    
    print(f"✅ Created: {output_path}")
    return True

# Create BlackRoad Mono fonts in multiple sizes
ttf_path = "/tmp/fonts/ttf/JetBrainsMono-Regular.ttf"
bold_path = "/tmp/fonts/ttf/JetBrainsMono-Bold.ttf"

if os.path.exists(ttf_path):
    # Regular weights
    create_vlw_font(ttf_path, "src/fonts/BlackRoadMono12.vlw", 12)
    create_vlw_font(ttf_path, "src/fonts/BlackRoadMono16.vlw", 16)
    create_vlw_font(ttf_path, "src/fonts/BlackRoadMono20.vlw", 20)
    create_vlw_font(ttf_path, "src/fonts/BlackRoadMono24.vlw", 24)
    
    # Bold
    if os.path.exists(bold_path):
        create_vlw_font(bold_path, "src/fonts/BlackRoadMonoBold16.vlw", 16, "BlackRoad Mono Bold")
        create_vlw_font(bold_path, "src/fonts/BlackRoadMonoBold20.vlw", 20, "BlackRoad Mono Bold")
    
    print("\n✅ All BlackRoad Mono fonts created!")
else:
    print(f"❌ Font not found: {ttf_path}")
