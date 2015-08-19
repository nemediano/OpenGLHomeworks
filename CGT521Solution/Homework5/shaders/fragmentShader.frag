#version 150
uniform float time;
uniform int fish_animation;
uniform int texture_map_flag;
uniform int texture_animation;
uniform int special_animation;
in vec2 text_coord;
uniform sampler2D texture_map;
out vec4 fragcolor;
void main(void){
   vec2 new_texture_coord = text_coord;
   float factor;
   if (texture_map_flag != 0) {
      fragcolor = texture(texture_map, new_texture_coord);
   } else if (texture_animation != 0) {
      new_texture_coord.x += 0.0225 * sin(time + new_texture_coord.x);
      new_texture_coord.y += 0.015 * sin(time);
      fragcolor = texture(texture_map, new_texture_coord);
   } else if (special_animation != 0) {
      factor = sin(4.0 * (time + new_texture_coord.x)) / 2.0 + 0.5;
      vec4 color_1 = vec4(1.0, 0.75, 0.0, 1.0);
      vec4 color_2 = vec4(0.9, 0.17, 0.31, 1.0);
      vec4 color_combination = factor * color_1 + (1.0 - factor) * color_2;
      vec4 color_texture = texture(texture_map, new_texture_coord);
      fragcolor = color_texture + (sin(4.0 * (time + new_texture_coord.x)) / 2.0 + 0.5) * color_combination;
   } else {
      fragcolor = vec4(0.85f, 0.85f, 0.85f, 1.0f);
   }
}
