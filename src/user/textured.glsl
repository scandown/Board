#version 330 core

out vec4 fragColour;
in vec2 uv;
in vec2 spr_num;

uniform sampler2D tex;
uniform bool is_atlas;

vec2 atlasSize = vec2(2.0, 1.0);

void main() {

	if (is_atlas) {
		vec2 spr_num_offset = spr_num / atlasSize;
		fragColour = texture(tex, uv / atlasSize + spr_num_offset);
	} else {
		fragColour = texture(tex, uv);
	}
}
