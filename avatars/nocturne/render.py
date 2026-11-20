"""Render three hand-keyed dance loops. Requires CairoSVG and Pillow."""
from io import BytesIO
from pathlib import Path
from sys import argv

import cairosvg
from PIL import Image

# Each pose controls hip x/y, torso tilt, head tilt, L shoulder/elbow, R shoulder/elbow.
# Angles for the two arms use mirrored local coordinates.
POSES = {
    "sway": [
        (-12, 2, 12, -11, 35, 49, 43, 78),
        (-16, 7, 15, -12, 29, 65, 36, 55),
        (-7, 1, 7, -6, 42, 73, 23, 60),
        (12, -1, -12, 11, 43, 78, 35, 49),
        (16, 7, -15, 12, 36, 55, 29, 65),
        (7, 2, -7, 6, 23, 60, 42, 73),
    ],
    "reach": [
        (-8, 3, 7, -5, 36, 46, 40, 70),
        (-16, 9, 14, -12, 63, 34, 35, 88),
        (-6, -4, 1, 4, 39, 65, 104, 33),
        (9, -8, -10, 10, 20, 47, 140, 18),
        (16, 0, -15, 7, 30, 58, 119, 15),
        (6, 5, -6, 0, 50, 50, 66, 58),
    ],
    "dip": [
        (5, -3, -4, 8, 33, 64, 46, 56),
        (10, 0, -9, 5, 51, 63, 58, 54),
        (-2, 9, 12, -1, 67, 14, 34, 90),
        (-17, 23, 28, -5, 79, -12, 24, 83),
        (-18, 19, 24, -16, 66, 11, 29, 60),
        (-4, 4, 7, 4, 46, 55, 48, 68),
    ],
}

BODY = """
<path fill="#141322" d="M-25-72 Q-15-76-11-79 L11-79 Q17-76 25-72
 Q28-64 18-48 Q13-36 16-22 L24-2 36 26 Q13 34-6 27 L-30 29 Q-26 5-19-16
 Q-13-34-19-44 Q-31-62-25-72Z"/>
<path fill="#302b58" stroke="none" d="M11-72 23-68 16-50 5-40 13-24 23-6 18 1
 4-16 0-30 7-50Z M-18-1 -5 4 -18 25 -26 26Z"/>
<path fill="#c4a8bc" d="M-10-86 11-86 12-75 6-65 1-55-4-61-13-76Z"/>
<path fill="#806581" stroke="none" d="M0-85 11-86 12-75 1-55 6-72Z"/>
<path fill="#0b0c16" d="M-15-73 -10-65 3-59-3-50-13-61-19-70Z"/>
<path fill="#5676e6" stroke="none" d="M14-42 8-35 14-23 7-26 15-13 13-21 18-20Z"/>
<path fill="#201c33" d="M-22-8 Q-2 0 23-6 L27 3 Q0 11-24 2Z"/>
<path fill="#d7cbec" d="M-8-3 7-2 7 7-8 6Z"/><path fill="#211d34" stroke="none" d="M-5 0 4 1 4 4-5 3Z"/>
<path fill="#aa99c8" stroke="none" d="M-18-3 -15-2-16 2-19 1Z M13 0 16-1 17 3 14 4Z M20-2 22-3 24 1 22 2Z"/>
<path fill="none" stroke="#51477e" stroke-width="1.2" d="M17 10 27 24 M-3 12 -9 23"/>
"""

HEAD = """
<path fill="#131522" d="M-19-4 Q-26-8-25-23 -25-30-18-37
 L-24-36 -13-46 -16-46 2-56 -2-47 14-59 10-48 23-54 19-42
 Q31-37 25-17 L19-4 9 5 -19 3Z"/>
<path fill="#293b8c" stroke="none" d="M-25-24 Q-22-38-11-43 L-13-39 2-51-1-43 15-54 11-45
 21-49 15-40 24-42 17-33 10-38-5-32-2-38-15-32-20-24Z"/>
<path fill="#6886fa" stroke="none" d="M-19-32 -8-43 -11-37 5-45 1-39 15-47 9-39 20-43 14-37
 3-35 5-39-7-34-4-38Z"/>
<path fill="#cfb9c5" d="M-16-17 Q-15-28-7-29 1-32 10-27 L16-18
 Q23-23 23-17 L18-9 14 1 7 10 0 12 -8 8 -15 1 -18-10Z"/>
<path fill="#9d7e9c" stroke="none" d="M14-22 17-18 21-19 19-13 15-9 13-1 7 8
 2 11-4 9 5 5 9-3 10-11Z M-15-18 -10-16-10-10-15-11Z"/>
<path fill="#181625" stroke="none" d="M-17-17 -17-26 -9-33 6-32 16-27 20-18 15-12
 11-23 9-24 9-17 3-23 2-27 -7-27 -12-22 -13-16Z"/>
<path fill="#41324c" stroke="none" d="M-13-12 -9-15-3-11-5-7-10-9Z M2-10 9-15 13-13 9-8 4-6Z"/>
<path fill="#171421" stroke="none" d="M-13-11 -8-10-4-8-9-8Z M3-8 9-10 13-12 10-8 5-6Z"/>
<path fill="#e9d9ed" stroke="none" d="M-10-10 -5-8 -8-8Z M5-8 10-9 7-7Z"/>
<path fill="none" stroke="#876d8a" stroke-width="1.2" d="M-2-8 -4-3 -1-2"/>
<path fill="#352139" stroke="none" d="M-10 0 Q-1 4 9-1 L6 5 1 7-6 5Z"/>
<path fill="#f3dce2" stroke="none" d="M-8 1 Q0 4 6 1 L4 4-2 5Z"/>
<path fill="none" stroke="#c6b5f5" stroke-width="1.4" d="M18-10 q4 4-1 7"/>
"""

UPPER = """
<path fill="{skin}" d="M-6-6 Q0-10 6-4 L7 13 5 33 Q1 39-5 34
 L-8 15 Q-10 4-6-6Z"/>
<path fill="{shade}" stroke="none" d="M-6-3 -4 11-2 26 4 34-3 35-7 18-8 5Z"/>
"""

FOREARM = """
<path fill="{skin}" d="M-5-5 Q0-7 5-3 L4 10 2 27 7 35 10 37 10 40
 6 39 1 35 3 44 1 45-3 36-4 44-6 44-7 33-9 40-11 39-9 28
 -5 24-7 10Q-8 0-5-5Z"/>
<path fill="{shade}" stroke="none" d="M-5-2 -3 7-2 22-6 28-8 30-7 25-7 10Z"/>
<path fill="#222032" d="M-6 22 3 23 2 27-6 26Z"/>
<path fill="#b8a5e7" stroke="none" d="M-3 23 0 23 0 25-3 25Z"/>
"""


def sample(poses, index, count):
    phase = index * len(poses) / count
    step = int(phase)
    t = phase - step
    before, start, end, after = (poses[i % len(poses)] for i in range(step - 1, step + 3))
    return [
        0.5 * ((2*b) + (-a+c)*t + (2*a-5*b+4*c-d)*t*t + (-a+3*b-3*c+d)*t*t*t)
        for a, b, c, d in zip(before, start, end, after)
    ]


def arm(side, shoulder, elbow, dark=False):
    skin = "#b599b2" if dark else "#d3bac7"
    shade = "#786582" if dark else "#9a7b98"
    palette = dict(skin=skin, shade=shade)
    return f'''<g transform="translate({side*24} -67) scale({side} 1) rotate({-shoulder})">
      {UPPER.format(**palette)}
      <g transform="translate(0 33) rotate({-elbow})">{FOREARM.format(**palette)}</g>
    </g>'''


def sprite(pose):
    x, y, tilt, head, ls, le, rs, re = pose
    left = arm(-1, ls, le, dark=True)
    right = arm(1, rs, re)
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 256 256">
      <g stroke="#10101b" stroke-width="1.65" stroke-linecap="round" stroke-linejoin="round"
         transform="translate({128+x} {176+y}) rotate({tilt})">
        {left}{BODY}{right}
        <g transform="translate(0 -89) rotate({head})">{HEAD}</g>
      </g>
    </svg>'''


def render(destination):
    count = 24
    previews = {}
    for name, poses in POSES.items():
        folder = destination / name
        folder.mkdir(parents=True, exist_ok=True)
        images = []
        for i in range(count):
            svg = sprite(sample(poses, i, count))
            raw = cairosvg.svg2png(bytestring=svg.encode(), output_width=768, output_height=768)
            frame = Image.open(BytesIO(raw)).convert("RGBA").resize((384, 384), Image.Resampling.LANCZOS)
            frame.save(folder / f"{i:04d}.png", optimize=True)
            images.append(frame.resize((160, 160), Image.Resampling.LANCZOS))
        previews[name] = images

    animation = []
    for i in range(count):
        strip = Image.new("RGBA", (480, 160), "#323146")
        for col, images in enumerate(previews.values()):
            strip.alpha_composite(images[i], (160 * col, 0))
        animation.append(strip.convert("RGB"))
    animation[0].save(destination / "preview.webp", save_all=True,
                      append_images=animation[1:], duration=[83, 83, 84] * 8, loop=0, quality=94)
    sheet = Image.new("RGBA", (160 * 6, 160 * 3), "#323146")
    for row, images in enumerate(previews.values()):
        for col in range(6):
            sheet.alpha_composite(images[col * 4], (160 * col, 160 * row))
    sheet.convert("RGB").save(destination / "poses.jpg", quality=94)


if __name__ == "__main__":
    render(Path(argv[1]))
