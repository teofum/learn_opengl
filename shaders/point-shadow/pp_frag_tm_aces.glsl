#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D screenTexture;

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat4 ACES_INPUT = mat4(
    0.59719, 0.35458, 0.04823, 0.0,
    0.07600, 0.90834, 0.01566, 0.0,
    0.02840, 0.13383, 0.83777, 0.0,
    0.0, 0.0, 0.0, 0.1
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat4 ACES_OUTPUT = mat4(
    1.60475, -0.53108, -0.07367, 0.0,
    -0.10208, 1.10813, -0.00605, 0.0,
    -0.00327, -0.07276, 1.07602, 0.0,
    0.0, 0.0, 0.0, 0.1
);

vec4 rrt_and_odt_fit(vec4 color) {
    vec4 a = color * (color + vec4(0.0245786)) - vec4(0.000090537);
    vec4 b = color * (color * 0.983729 + vec4(0.4329510)) + vec4(0.238081);
    return vec4(a[0] / b[0], a[1] / b[1], a[2] / b[2], 0.0);
}

void main() {
    vec4 hdr = texture(screenTexture, texCoord);
    vec4 color = hdr * ACES_INPUT;
    color = rrt_and_odt_fit(color);
    color = color * ACES_OUTPUT;

    clamp(vec4(0.0), vec4(1.0), color);
    FragColor = vec4(color.rgb, 1.0);
}
