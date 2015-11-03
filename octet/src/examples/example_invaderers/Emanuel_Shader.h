

namespace octet {
	namespace shaders {
		class emanuel_shader : public shader {
			// indices to use with glUniform*()

			// index for model space to projection space matrix
			GLuint modelToProjectionIndex_;


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
				void main() { 
						gl_FragColor = vec4(1.0, 0.0, 0.0, 0.1); 
					}
				);

				// use the common shader code to compile and link the shaders
				// the result is a shader program
				shader::init(vertex_shader, fragment_shader);

				// extract the indices of the uniforms to use later
				modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
			}

			void render(const mat4t &modelToProjection) {
				// tell openGL to use the program
				shader::render();

				// customize the program with uniforms
				glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());
			}
		};
	}
}
