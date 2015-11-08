

namespace octet {
	namespace shaders {
		class emanuel_shader : public shader {

			// index for model space to projection space matrix
			GLuint modelToProjectionIndex_;
			GLuint resolutionIndex_;


		public:
			void init() {

				const char vertex_shader[] = SHADER_STR(


				attribute vec4 pos;
				uniform mat4 modelToProjection;

				void main() { gl_Position = modelToProjection * pos; 
					}
				);

				// this is the fragment shader
				// after the rasterizer breaks the triangle into fragments
				// this is called for every fragment
				// it outputs gl_FragColor, the color of the pixel and inputs uv_
				const char fragment_shader[] = SHADER_STR(
					uniform vec2 resolution;
				void main() { 
					vec2 pos = gl_FragCoord.xy / resolution.xy;
					vec3 color = vec3(0.1, 0.1, 0.6) * pos.y;
						gl_FragColor = vec4(color, 1.0); 
					}
				);

				// use the common shader code to compile and link the shaders
				// the result is a shader program
				shader::init(vertex_shader, fragment_shader);

				// extract the indices of the uniforms to use later
				modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
				resolutionIndex_ = glGetUniformLocation(program(), "resolution");
			}

			void render(const mat4t &modelToProjection, const vec2 &resolution) {
				// tell openGL to use the program
				shader::render();

				// customize the program with uniforms
				glUniform2fv(resolutionIndex_, 1, resolution.get());
				glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());
			}
		};
	}
}
