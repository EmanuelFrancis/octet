////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// invaderer example: simple game with sprites and sounds
//
// Level: 1
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Simple game mechanics
//   Texture loaded from GIF file
//   Audio
//

namespace octet {
	class sprite {
		// where is our sprite (overkill for a 2D game!)
		mat4t modelToWorld;

		// half the width of the sprite
		float halfWidth;

		// half the height of the sprite
		float halfHeight;

		// what texture is on our sprite
		int texture;

		// true if this sprite is enabled.
		bool enabled;
	public:

		float angle = 0;

		sprite() {
			texture = 0;
			enabled = true;
		}

		vec2 getxy() {
			return modelToWorld.row(3).xy();
		}

		void init(int _texture, float x, float y, float w, float h) {
			modelToWorld.loadIdentity();
			modelToWorld.translate(x, y, 0);
			halfWidth = w * 0.5f;
			halfHeight = h * 0.5f;
			texture = _texture;
			enabled = true;
		}

		void swap_texture(int _texture) {
			texture = _texture;
		}

		void render(texture_shader &shader, mat4t &cameraToWorld) {
			// invisible sprite... used for gameplay.
			if (!texture) return;

			// build a projection matrix: model -> world -> camera -> projection
			// the projection space is the cube -1 <= x/w, y/w, z/w <= 1
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

			// set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);

			// use "old skool" rendering
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			shader.render(modelToProjection, 0);

			// this is an array of the positions of the corners of the sprite in 3D
			// a straight "float" here means this array is being generated here at runtime.
			float vertices[] = {
			  -halfWidth, -halfHeight, 0,
			   halfWidth, -halfHeight, 0,
			   halfWidth,  halfHeight, 0,
			  -halfWidth,  halfHeight, 0,
			};

			// attribute_pos (=0) is position of each corner
			// each corner has 3 floats (x, y, z)
			// there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vertices);
			glEnableVertexAttribArray(attribute_pos);

			// this is an array of the positions of the corners of the texture in 2D
			static const float uvs[] = {
			   0,  0,
			   1,  0,
			   1,  1,
			   0,  1,
			};

			// attribute_uv is position in the texture of each corner
			// each corner (vertex) has 2 floats (x, y)
			// there is no gap between the 2 floats and hence the stride is 2*sizeof(float)
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)uvs);
			glEnableVertexAttribArray(attribute_uv);

			// finally, draw the sprite (4 vertices)
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
	
		//Use Emanuel Shader
		void render(emanuel_shader &shader, mat4t &cameraToWorld, int v_width, int v_height) {
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

			shader.render(modelToProjection, vec2(v_width, v_height));

			float vertices[] = {
				-halfWidth, -halfHeight, 0,
				halfWidth, -halfHeight, 0,
				halfWidth, halfHeight, 0,
				-halfWidth, halfHeight,0,

			};

			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vertices);
			glEnableVertexAttribArray(attribute_pos);

			static const float uvs[] = {
				0, 0,
				1, 0,
				1, 1,
				0, 1,
			};

			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)uvs);
			glEnableVertexAttribArray(attribute_uv);

			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}

	


    // move the object
    void translate(float x, float y) {
      modelToWorld.translate(x, y, 0);
    }

	//rotate attempt!
	void rotateZ(float angle) {
		modelToWorld.rotateZ(angle);
	}



    // position the object relative to another.
    void set_relative(sprite &rhs, float x, float y) {
      modelToWorld = rhs.modelToWorld;
      modelToWorld.translate(x, y, 0);
    }

    // return true if this sprite collides with another.
    // note the "const"s which say we do not modify either sprite
    bool collides_with(const sprite &rhs) const {
      float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];
      float dy = rhs.modelToWorld[3][1] - modelToWorld[3][1];

      // both distances have to be under the sum of the halfwidths
      // for a collision
      return
        (fabsf(dx) < halfWidth + rhs.halfWidth) &&
        (fabsf(dy) < halfHeight + rhs.halfHeight)
      ;
    }

    bool is_above(const sprite &rhs, float margin) const {
      float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];

      return
        (fabsf(dx) < halfWidth + margin)
      ;
    }

    bool &is_enabled() {
      return enabled;
    }
  };

  class invaderers_app : public octet::app {
	  // Matrix to transform points in our camera space to the world.
	  // This lets us move our camera
	  mat4t cameraToWorld;

	  // shader to draw a textured triangle
	  texture_shader texture_shader_;
	  emanuel_shader emanuel_shader_;

	  enum {
		  num_sound_sources = 8,
		  num_missiles = 6,
		  num_bombs = 2,
		  num_borders = 4,

		  // sprite definitions
		  ship_sprite = 0,
		  game_over_sprite,

		  first_missile_sprite,
		  last_missile_sprite = first_missile_sprite + num_missiles - 1,

		  first_bomb_sprite,
		  last_bomb_sprite = first_bomb_sprite + num_bombs - 1,

		  first_border_sprite,
		  last_border_sprite = first_border_sprite + num_borders - 1,

		  explosion_sprite,

		  powerup01_sprite,

		  num_sprites,
	  };

	  // timers for missiles and bombs
	  int missiles_disabled;
	  int bombs_disabled;

	  // accounting for bad guys
	  int live_invaderers;
	  int num_lives;

	  // game state
	  bool game_over;
	  int score;
	  int powerup_count = 0;

	  // speed of enemy
	  float invader_velocity;

	  // sounds
	  ALuint whoosh;
	  ALuint bang;
	  unsigned cur_source;
	  ALuint sources[num_sound_sources];

	  // big array of sprites
	  sprite sprites[num_sprites];
	  dynarray<sprite> inv_sprites;
	  sprite background_sprite;

	  // random number generator
	  class random randomizer;

	  // a texture for our text
	  GLuint font_texture;

	  // information for our text
	  bitmap_font font;

	  // stores current level
	  int current_level = 0;
	  static const int MAX_NR_LVL = 2;

	  // invanderer position
	  struct inv_position {
		  int x;
		  int y;
	  };

	  dynarray<inv_position> inv_formation;

	  // missile power ups
	  int powerup_sprite_no = 0;
	  GLint powerup_texture[3] = {
	  };

	  // missile x and y trajectorys
	  int missile_y = 5;
	  int missile_x = 5;

	  float missile_trajectory_angle = 0;
	  float missile_rotation = 0;

	  bool double_missiles = false;


	  ALuint get_sound_source() { return sources[cur_source++ % num_sound_sources]; }


	  void clear_screen() {                           	//Actually, this func does not clear,
		  COORD coord = { 0 };                                //it just set the cursor at the
		  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);    // top left corner of the screen
		  SetConsoleCursorPosition(h, coord);
	  }

	  void update_powerups() {
		  GLuint missile = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/missile.gif");
		  GLuint second_powerup = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/powerup_02.gif");
		  GLuint first_powerup = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/powerup_01.gif");
		  powerup_texture[0] = missile;
		  powerup_texture[1] = first_powerup;
		  powerup_texture[2] = second_powerup;
		  for (int i = 0; i != num_missiles; ++i) {
			  // create missiles off-screen
			  //  sprites[first_missile_sprite + i].swap_texture(first_powerup);
			  if (powerup_sprite_no == 0) {
				  sprites[first_missile_sprite + i].swap_texture(powerup_texture[0]);
				  double_missiles = false;
				  missile_rotation = 0;
				  missile_trajectory_angle = 0;
				  sprites[first_missile_sprite + i].angle = 0;
			  }
			  if (powerup_count > 3 && powerup_count < 8) {
				  powerup_sprite_no = 1;
			  }
			  if (powerup_sprite_no == 1) {
				  sprites[first_missile_sprite + i].swap_texture(powerup_texture[1]);
				  double_missiles = false;
				  missile_rotation = 0;
				  missile_trajectory_angle = 0;
				  sprites[first_missile_sprite + i].angle = 0;
			  }
			  if (powerup_count > 7) {
				  powerup_sprite_no = 2;
			  }
			  if (powerup_sprite_no == 2) {
				  sprites[first_missile_sprite + i].swap_texture(powerup_texture[2]);
				  missile_trajectory_angle = 0.02f;
				  missile_rotation = 1;
				  double_missiles = true;
			  }
		  }
		  
	  }

    // called when we hit an enemy
      void on_hit_invaderer() {
      ALuint source = get_sound_source();
      alSourcei(source, AL_BUFFER, bang);
      alSourcePlay(source);

      live_invaderers--;
      score++;
	  powerup_count++;

	  if (live_invaderers == 4) {
		  invader_velocity *= 4;
      }
	  if (live_invaderers == 0 && current_level <= MAX_NR_LVL) {
		  load_next_level();
	  } 
    }

    // called when we are hit
    void on_hit_ship() {
      ALuint source = get_sound_source();
      alSourcei(source, AL_BUFFER, bang);
      alSourcePlay(source); 

	  if (powerup_sprite_no != 0) {
		  --powerup_sprite_no;

		  if (powerup_sprite_no == 0) {
			  powerup_count = 0;
		  }
		  else if (powerup_sprite_no == 1) {
			  powerup_count = 3;
		  }
	  }

      if (--num_lives == 0) {
        game_over = true;
        sprites[game_over_sprite].translate(-20, 0);
      }

    }

    // use the keyboard to move the ship
	void move_ship() {
		const float ship_speed = 0.05f;
		// left and right arrows
		if (is_key_down(key_left)) {
			sprites[ship_sprite].translate(-ship_speed, 0);
			if (sprites[ship_sprite].collides_with(sprites[first_border_sprite + 2])) {
				sprites[ship_sprite].translate(+ship_speed, 0);
			}
		}
		else if (is_key_down(key_right)) {
			sprites[ship_sprite].translate(+ship_speed, 0);
			if (sprites[ship_sprite].collides_with(sprites[first_border_sprite + 3])) {
				sprites[ship_sprite].translate(-ship_speed, 0);
			}
		}
		if (is_key_down(key_up)) {
			sprites[ship_sprite].translate(0, +ship_speed);
			if (sprites[ship_sprite].collides_with(sprites[first_border_sprite + 1])) {
				sprites[ship_sprite].translate(0, -ship_speed);
			}
		}
		if (is_key_down(key_down)) {
			sprites[ship_sprite].translate(0, -ship_speed);
			if (sprites[ship_sprite].collides_with(sprites[first_border_sprite])) {
				sprites[ship_sprite].translate(0, +ship_speed);
			}
		}
	}


    // fire button (space) 
    void fire_missiles() {
      if (missiles_disabled) {                        // if missiles are available (if missiles_disabled has a value above 0) 
        --missiles_disabled;						  // reset them to 0 (take them all away)
	  }
	  else 
		  if (is_key_going_down(' ')) {			      // otherwise if space is hit (shot fired)

	  
        // find a missile								                                   
        for (int i = 0; i != num_missiles; ++i) {		                                   // when the counter has not yet add up to the number of missiles allowed
          if (!sprites[first_missile_sprite+i].is_enabled()) {							   // if the first missile sprite is not visible (default set as not visible when texture is loaded) 
            sprites[first_missile_sprite+i].set_relative(sprites[ship_sprite], 0, 0.5f);   // move missile sprite relative to ship (still invisible)
            sprites[first_missile_sprite+i].is_enabled() = true;
	//		sprites[first_missile_sprite + i].angle = 0;                                    // make the first missile sprite visible
            missiles_disabled = 5;															// make 5 available missiles
            ALuint source = get_sound_source();												// go through the array of sound sources
            alSourcei(source, AL_BUFFER, whoosh);											// find the whoosh sound
            alSourcePlay(source);															// play the found sound
            break;																			// stop when finished
          }
        }

		if (double_missiles == true) {
			for (int i = 0; i != num_missiles; ++i) {		                                            
  				if (!sprites[first_missile_sprite + i].is_enabled()) {							         
					sprites[first_missile_sprite + i].set_relative(sprites[ship_sprite], 0.5f, 0.5f);   
					sprites[first_missile_sprite + i].is_enabled() = true;
					sprites[first_missile_sprite + i].angle = -1;                                       
					missiles_disabled = 5;															    

					ALuint source = get_sound_source();												    
					alSourcei(source, AL_BUFFER, whoosh);											    
					alSourcePlay(source);															    
					break;																			    
				}
			}

			for (int i = 0; i != num_missiles; ++i) {		                                   
				if (!sprites[first_missile_sprite + i].is_enabled()) {							    
					sprites[first_missile_sprite + i].set_relative(sprites[ship_sprite], -0.5f, 0.5f);   
					sprites[first_missile_sprite + i].is_enabled() = true;
					sprites[first_missile_sprite + i].angle = 1;                                    
					missiles_disabled = 5;															
					ALuint source = get_sound_source();												
					alSourcei(source, AL_BUFFER, whoosh);											
					alSourcePlay(source);															
					break;																			
				}
			}
		}

      }
    }

    // pick and invader and fire a bomb
    void fire_bombs() {
      if (bombs_disabled) {
        --bombs_disabled;
      } else {
        // find an invaderer
        sprite &ship = sprites[ship_sprite];
        for (int j = randomizer.get(0, inv_sprites.size()); j < inv_sprites.size(); ++j) {
          sprite &invaderer = inv_sprites[j];
          if (invaderer.is_enabled() && invaderer.is_above(ship, 0.3f)) {
            // find a bomb
            for (int i = 0; i != num_bombs; ++i) {
              if (!sprites[first_bomb_sprite+i].is_enabled()) {
                sprites[first_bomb_sprite+i].set_relative(invaderer, 0, -0.25f);
                sprites[first_bomb_sprite+i].is_enabled() = true;
                bombs_disabled = 30;
                ALuint source = get_sound_source();
                alSourcei(source, AL_BUFFER, whoosh);
                alSourcePlay(source);
                return;
              }
            }
            return;
          }
        }
      }
    }


    // animate the missiles
    void move_missiles() {                                      
	    static int count = 0;                                      // set counter for frame counting 
		const float xangle = missile_trajectory_angle;
		static bool active = false;                                // set an on/off explosion frame counting switch
        const float missile_speed = 0.2;                           // set missile speed

	  if (active) {                                                // if explosion frame counting switch is on
		  count++;                                                 // +1 to counter
	  }
	  if (count >= 5 && active) {                                  // if 5 frames have passed and explosion frame counting switch is (true?)
			  sprites[explosion_sprite].translate(50, 50);         // move explosion sprite off screen
			  active = false;                                      // turn off explosion frame counting switch
			  count = 0;                                           // reset counter to 0
		  }
      for (int i = 0; i != num_missiles; ++i) {					                  // if we have not reached the missile limit     
        sprite &missile = sprites[first_missile_sprite+i];		                  // define a sprite object and assign the missile sprite to it
        if (missile.is_enabled()) {								                  // if missile object is enabled
          missile.translate(xangle, missile_speed);				                  // move the missile in the angle defined by xangle, and to the speed defined by missile_speed
		  missile.rotateZ(sprites[first_missile_sprite + i].angle);				  // with the rotation in the z defined by missile_rotation
          for (int j = 0; j != inv_sprites.size(); ++j) {				          //when there are still invaderers enabled
            sprite &invaderer = inv_sprites[j];		                              // define a sprite object and assign the invaderers sprite to it 
            if (invaderer.is_enabled() && missile.collides_with(invaderer)) {     // if invaderer is alive and missile collides
              invaderer.is_enabled() = false;                                     // turn off / kill invaderer
			  sprites[explosion_sprite].set_relative(invaderer, 0, 0);            // move explosion sprite relative to invaderer
			  active = true;                                                      // turn on explosion frame counting switch

              invaderer.translate(20, 0);
              missile.is_enabled() = false;

              missile.translate(20, 0);
              on_hit_invaderer();

              goto next_missile;
            }
          }

          if (missile.collides_with(sprites[first_border_sprite+1]) || missile.collides_with(sprites[first_border_sprite + 2]) || missile.collides_with(sprites[first_border_sprite + 3])) {
            missile.is_enabled() = false;
            missile.translate(20, 0);
          }
        }

      next_missile:;
      }
    }

    // animate the bombs
    void move_bombs() {
      const float bomb_speed = 0.2f;
      for (int i = 0; i != num_bombs; ++i) {
        sprite &bomb = sprites[first_bomb_sprite+i];
        if (bomb.is_enabled()) {
          bomb.translate(0, -bomb_speed);
          if (bomb.collides_with(sprites[ship_sprite])) {
            bomb.is_enabled() = false;
            bomb.translate(20, 0);
            bombs_disabled = 50;
            on_hit_ship();
            goto next_bomb;
          }
          if (bomb.collides_with(sprites[first_border_sprite+0])) {
            bomb.is_enabled() = false;
            bomb.translate(20, 0);
          }
        }
      next_bomb:;
      }
    }

    // move the array of enemies
    void move_invaders(float dx, float dy) {
      for (int j = 0; j != inv_sprites.size(); ++j) {
        sprite &invaderer = inv_sprites[j];
        if (invaderer.is_enabled()) {
          invaderer.translate(dx, dy);
        }
      }
    }

    // check if any invaders hit the sides.
    bool invaders_collide(sprite &border) {
      for (int j = 0; j != inv_sprites.size(); ++j) {
        sprite &invaderer = inv_sprites[j];
        if (invaderer.is_enabled() && invaderer.collides_with(border)) {
          return true;
        }
      }
      return false;
    }


    void draw_text(texture_shader &shader, float x, float y, float scale, const char *text) {
      mat4t modelToWorld;
      modelToWorld.loadIdentity();
      modelToWorld.translate(x, y, 0);
      modelToWorld.scale(scale, scale, 1);
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      /*mat4t tmp;
      glLoadIdentity();
      glTranslatef(x, y, 0);
      glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);
      glScalef(scale, scale, 1);
      glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);*/

      enum { max_quads = 32 };
      bitmap_font::vertex vertices[max_quads*4];
      uint32_t indices[max_quads*6];
      aabb bb(vec3(0, 0, 0), vec3(256, 256, 0));

      unsigned num_quads = font.build_mesh(bb, vertices, indices, max_quads, text, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, font_texture);

      shader.render(modelToProjection, 0);

      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].x );
      glEnableVertexAttribArray(attribute_pos);
      glVertexAttribPointer(attribute_uv, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].u );
      glEnableVertexAttribArray(attribute_uv);

      glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, indices);
    }

  public:

    // this is called when we construct the class
    invaderers_app(int argc, char **argv) : app(argc, argv), font(512, 256, "assets/big.fnt") {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shader
      texture_shader_.init();
	  emanuel_shader_.init();

      // set up the matrices with a camera 5 units from the origin
      cameraToWorld.loadIdentity();
      cameraToWorld.translate(0, 0, 3);

      font_texture = resource_dict::get_texture_handle(GL_RGBA, "assets/big_0.gif");


      GLuint ship = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/ship.gif");
      sprites[ship_sprite].init(ship, 0, -2.75f, 0.25f, 0.25f);

	  GLuint explosion = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/explosion.gif");
	  sprites[explosion_sprite].init(explosion, -1000, -1000, 0.25f, 0.25f);

      GLuint GameOver = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/GameOver.gif");
      sprites[game_over_sprite].init(GameOver, 20, 0, 3, 1.5f);


      // set the border to white for clarity
      GLuint white = resource_dict::get_texture_handle(GL_RGB, "#ffffff");
      sprites[first_border_sprite+0].init(white, 0, -3, 6, 0.2f);
      sprites[first_border_sprite+1].init(white, 0,  3, 6, 0.2f);
      sprites[first_border_sprite+2].init(white, -3, 0, 0.2f, 6);
      sprites[first_border_sprite+3].init(white, 3,  0, 0.2f, 6);
	  background_sprite.init(white, 0, 0, 6, 6);
	

      // use the bomb texture
      GLuint bomb = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/bomb.gif");
      for (int i = 0; i != num_bombs; ++i) {
        // create bombs off-screen
        sprites[first_bomb_sprite+i].init(bomb, 20, 0, 0.0625f, 0.25f);
        sprites[first_bomb_sprite+i].is_enabled() = false;
      }

	  // use the missile texture
	  GLuint missile = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/missile.gif");         // create missile sprite from texture library and name it missile
	  if (powerup_sprite_no == 0) {
		  for (int i = 0; i != num_missiles; ++i) {																// if th counter does not equal the amount of allowed missiles	
			  // create missiles off-screen
			  sprites[first_missile_sprite + i].init(missile, 20, 0, 0.0625f, 0.25f);							// load the missile into the game off screen
	     	  sprites[first_missile_sprite + i].is_enabled() = false;											// set is as invisible (disabled) by default
		  }
	  }

	  load_next_level();

      // sounds
      whoosh = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/whoosh.wav");
      bang = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/bang.wav");
      cur_source = 0;
      alGenSources(num_sound_sources, sources);

      // sundry counters and game state.
      missiles_disabled = 0;
      bombs_disabled = 50;
      invader_velocity = 0.01f;
      live_invaderers = inv_sprites.size();
      num_lives = 10;
      game_over = false;
      score = 0;
	  powerup_sprite_no = 0;

    }

	

    // called every frame to move things
	
    void simulate() {


      if (game_over) {

		  return;
	  
      }

      move_ship();

	  update_powerups();

      fire_missiles();

      fire_bombs();

      move_missiles();

      move_bombs();

      move_invaders(invader_velocity, 0);

      sprite &border = sprites[first_border_sprite+(invader_velocity < 0 ? 2 : 3)];
      if (invaders_collide(border)) {
        invader_velocity = -invader_velocity;
        move_invaders(invader_velocity, -0.1f);
      }


    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {

      simulate();

      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      // clear the background to black
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glDisable(GL_DEPTH_TEST);

      // allow alpha blend (transparency when alpha channel is 0)
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	  // draw background
	  background_sprite.render(emanuel_shader_, cameraToWorld, w, h);

	  // new draw sprites
	  for (int i = 0; i < inv_sprites.size(); ++i) {
		  inv_sprites[i].render(texture_shader_, cameraToWorld);
	  }

      // draw all andy's sprites
      for (int i = 0; i != num_sprites; ++i) {
        sprites[i].render(texture_shader_, cameraToWorld);
      }

      char score_text[32];
      sprintf(score_text, "score: %d   lives: %d", score, num_lives);
      draw_text(texture_shader_, -1.75f, 2, 1.0f/256, score_text);

      // move the listener with the camera
      vec4 &cpos = cameraToWorld.w();
      alListener3f(AL_POSITION, cpos.x(), cpos.y(), cpos.z());
    }

	// read .csv file
	void read_file() {

		std::ifstream file("inv_formation" + std::to_string(current_level) + ".csv");

		inv_formation.resize(0);

		char buffer[2048];
		int i = 0;
		while (!file.eof()) {
			file.getline(buffer, sizeof(buffer));

			char *b = buffer;
			for (int j = 0;; ++j) {
				char *e = b;
				while (*e != 0 && *e != ',') {
					++e;
				}

				if (std::atoi(b) == 1) {
					inv_position p;
					p.x = j;
					p.y = i;
					inv_formation.push_back(p);
				}

				if (*e != ',') {
					break;
				}
				b = e +  1; 
			}
			++i;
		}
	}

	// load next level
	void load_next_level() {

		++current_level;
		live_invaderers = inv_sprites.size();
		if (current_level > MAX_NR_LVL) {
			game_over = true;
			sprites[game_over_sprite].translate(-20, 0);
		}

		// display invaderer formation
		read_file();
		inv_sprites.resize(0);

		GLuint invaderer = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/invaderer.gif");
		for (int i = 0; i < inv_formation.size(); ++i) {
			sprite inv;
			inv.init(invaderer, -1.5f + 0.66f*inv_formation[i].x, 2 - 0.5f*inv_formation[i].y, 0.25f, 0.25f);
			inv_sprites.push_back(inv);
		}
	}
  };
}
