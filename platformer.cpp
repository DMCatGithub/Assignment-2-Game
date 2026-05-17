#include <engine.h>
#include <algorithm>
#include <iostream>
#include <vector>

// Simple structure to store an animation
struct Animation {
    std::vector<Texture> frames; // A vector (sequence) of individual frames
    int no_frames;               // Number of frames (just the length of the vector)
    float duration;              // Duration of animation
    bool loop;                   // Loop (does the animation loop or just play once)
    float start;
};

// List of Animation
std::vector<Animation> animations;

// Animation State
enum AnimState {IDLE, RUNNING, JUMPING, FALLING};

// Player
class Player {
    public:
    Vec2 pos;
    Vec2 vel;
    Vec2 size;
    bool isStanding;
    AnimState state;
};

int player_max_vel_x;
int player_min_vel_x;
int player_min_max_vel_x;

int player_max_vel_y;
int player_min_vel_y;
int player_min_max_vel_y;

// Platform
class Platform {
    public:
    Vec2 pos;
    Vec2 size;
};

// Player
Player player;

// Platforms
std::vector<Platform> platforms;

// Gravity
float gravity_time;     // Timer for gravity cycle
float gravity_period;   // Full cycle of hi>>low>>hi gravity
float gravity_theta;    // Angle for cosine wave
float gravity_phase;    // Phase of gravity (0 to 1)
float gravity_max;      // Maximum gravity
float gravity_min;      // Minimum gravity
float gravity;          // Current gravity

int gravity_background_colour = 0; // Background colour based on gravity

// Scrolling
float screen_scroll_offset = 0.0f;
float screen_center = WINDOW_WIDTH / 2.0f;
Vec2 screen_pos = Vec2::zero;

// Starfield
int star_count;
struct Star {float x; float y; float size; float star_twinkle_timer;};
std::vector<Star> stars;
int level_width;
float star_off_time;


Vec2 screen_star_pos;

// Earth
Texture earth_texture;
Vec2 earth_size;

Vec2 earth_pos;
Vec2 earth_screen_pos = earth_pos;

Vec2 earth_orbit_center = Vec2::zero;
int earth_orbit_radius_x;
int earth_orbit_radius_y;

float earth_time = 0.0f;                    // Timer for gravity cycle
float earth_period = gravity_period*2;      // Full cycle of hi>>low>>hi gravity
float earth_theta = 0.0f;
float earth_phase_shift = 0.0f;              // Phase shift to correct earth orbit position





// Check for collisions between player and platform
void collisions(Player &player, Platform &platform) {
    // Bounds of player & platform
    float playerLeft   = player.pos.x - player.size.x/2;
    float playerRight  = player.pos.x + player.size.x/2;
    float playerTop    = player.pos.y - player.size.y/2;
    float playerBottom = player.pos.y + player.size.y/2;

    float platformLeft   = platform.pos.x - platform.size.x/2; 
    float platformRight  = platform.pos.x + platform.size.x/2;
    float platformTop    = platform.pos.y - platform.size.y/2;
    float platformBottom = platform.pos.y + platform.size.y/2;

    // Check if player and platform are colliding
    if((playerLeft < platformRight) && (playerRight > platformLeft) &&
        (playerTop < platformBottom) && (playerBottom > platformTop)) {
        // Process Collision
        double tx = 1000000;
        double ty = 1000000;

        // If player is going left
        if(player.vel.x < 0) {
            // Calculate time since collision in X
            tx = (playerLeft - platformRight) / player.vel.x;
        } else if(player.vel.x > 0) {
            // Calculate time since collision in X
            tx = (playerRight - platformLeft) / player.vel.x;
        }

        // If player is going up
        if(player.vel.y < 0) {
            // Calculate time since collision in Y
            ty = (playerTop - platformBottom) / player.vel.y;
        } else if(player.vel.y > 0) {
            // Calculate time since collision in Y
            ty = (playerBottom - platformTop) / player.vel.y;
        }

        // Work out which collision to apply
        if(tx < ty) {
            // Collided horizontally first
            if(player.vel.x < 0) {
                // Stop player moving
                player.vel.x = 0;

                // Correct position
                player.pos.x = platformRight + player.size.x/2;
            } else {
                // Stop player moving
                player.vel.x = 0;

                // Correct position
                player.pos.x = platformLeft - player.size.x/2;
            }
        } else {
            // Collided vertically first
            if(player.vel.y < 0) {
                // Stop player moving
                player.vel.y = 0;

                // Correct position
                player.pos.y = platformBottom + player.size.y/2;
            } else {
                // Stop player moving
                player.vel.y = 0;
                
                // Correct position
                player.pos.y = platformTop - player.size.y/2;

                // Player is standing
                player.isStanding = true;
            }
        }
    }
}

// Load Animation (from series of files)
Animation loadAnimation(const char *file_base, int no_frames, float duration, bool loop) {
    // File name
    char file[1024];

    // Animation
    Animation result;

    // Load frames
    for(int i = 0; i < no_frames; i++) {
        snprintf(file, 1024, "%s%03d.png", file_base, i);
        result.frames.push_back(loadTexture(file));
    }

    // Set parameters
    result.no_frames = result.frames.size();
    result.duration = duration;
    result.loop = loop;

    // Return
    return result;
}


// Initialise (called once at start)
void init() {
    setWindowTitle("Platformer");

    // Load animations
    animations.push_back(loadAnimation("./assets/images/Idle__", 10, 1.0f, true));
    animations.push_back(loadAnimation("./assets/images/Run__", 10, 0.5f, true));
    animations.push_back(loadAnimation("./assets/images/Jump__", 10, 0.5f, false));
    animations.push_back(Animation {std::vector<Texture>({loadTexture("./assets/images/Jump__009.png")}), 1, 1.0f, true});

    // Create Player
    player.pos = Vec2(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    player.vel = Vec2::zero;
    player.size = Vec2(64, 96);

    player_max_vel_x = 200;
    player_min_vel_x = 100;
    player_min_max_vel_x = player_max_vel_x - player_min_vel_x;

    player_max_vel_y = -300;
    player_min_vel_y = -100;
    player_min_max_vel_y = player_max_vel_y - player_min_vel_y;


    // Create platforms
    level_width = 4000;
    platforms.push_back(Platform{Vec2(level_width/2, WINDOW_HEIGHT - 16), Vec2(level_width, 32)});

    platforms.push_back(Platform{Vec2(WINDOW_WIDTH/2, WINDOW_HEIGHT - 100), Vec2(96, 32)});
    platforms.push_back(Platform{Vec2(WINDOW_WIDTH/2 + 300, WINDOW_HEIGHT - 300), Vec2(96, 32)}); 
    platforms.push_back(Platform{Vec2(WINDOW_WIDTH/2 + 500, WINDOW_HEIGHT - 500), Vec2(96, 32)});



    // Create starfield
    // level_width = 4000;
    screen_star_pos = Vec2::zero;
    star_off_time = 0.2f; // Time for star to be off during twinkle

    star_count = 200;
    for(int i = 0; i < star_count; i++) {
        Star star;
        star.x = uniform(0, level_width + WINDOW_WIDTH/2); // Screen scrolls after 1/2 window_width
        star.y = uniform(0, WINDOW_HEIGHT);
        star.size = uniform(1, 3);
        star.star_twinkle_timer = 0; // Randomly set star on or off for twinkling effect
        stars.push_back(star);
    }   

    // Earth
    earth_texture = loadTexture("./assets/images/spritesheet.png",Vec2(720,0),Vec2(240,240));
    earth_size = Vec2(200, 200);

    earth_pos = Vec2::zero;
    earth_screen_pos = earth_pos;

    earth_orbit_center = Vec2(level_width/2 - earth_size.x/2, WINDOW_HEIGHT);
    earth_orbit_radius_x = level_width/2 + earth_size.x;
    earth_orbit_radius_y = WINDOW_HEIGHT;
    earth_period = gravity_period * 2;      // gravity_period is light-dark // earth_period is light-dark-light

    earth_time = 0.0f;
    earth_period = gravity_period*2;
    earth_theta = 0.0f;
    earth_phase_shift = 0.0f;

    // Gravity
    gravity_time = 0.0f;          // Timer for gravity cycle
    gravity_period = 20.0f;      // Full cycle of hi>>low>>hi gravity
    gravity_theta = 0.0f;    // Angle for cosine wave
    gravity_phase = 0.0f;                    // Phase of gravity (0 to 1)
    gravity_max = 981;            // Maximum gravity
    gravity_min = 160;            // Minimum gravity
    gravity = 0.0f; // Current gravity

    gravity_background_colour = 0; // Background colour based on gravity



}

// Update Game
void update(float dt) {

    // Gravity
    gravity_time += dt;         // Timer for gravity cycle
    if (gravity_time > 2 * gravity_period) {
        gravity_time = 0;       // Reset Timer
    }
    gravity_theta = (2.0f * M_PI * gravity_time) / gravity_period;    // Angle for cosine wave
    gravity_phase = (cosf(gravity_theta) + 1) / 2;                    // Phase of gravity (0 to 1)
    gravity = gravity_min + gravity_phase * (gravity_max - gravity_min); // Current gravity
    gravity_background_colour = (int)(255.0f * (gravity_phase)); // Background colour based on gravity
 
    player.vel.x = 0;

    // Scrolling
    screen_scroll_offset = player.pos.x - screen_center;
    if (screen_scroll_offset < 0) {
        screen_scroll_offset = 0; // Prevent walking past the left edge
    }
    
    if (screen_scroll_offset > level_width - WINDOW_WIDTH) {
        screen_scroll_offset = level_width - WINDOW_WIDTH; // Prevent walking past the right edge
    }

    // Starfield /// WORK ON THIS
    for(int i = 0; i < star_count; i++) {
        stars[i].star_twinkle_timer -= dt;
        if (stars[i].star_twinkle_timer > 0) {
            continue;
        }

        if (stars[i].star_twinkle_timer < 0) {
            stars[i].star_twinkle_timer = 0;
        }

        if (uniform(0,1) < 0.001f) {
                stars[i].star_twinkle_timer = star_off_time;
            }
        }
    



    // Earth ellipctical orbit
    earth_time = gravity_time;         // Timer for earth orbit
    earth_theta = (2.0f * M_PI * earth_time) / gravity_period;    // Angle for cosine wave
    earth_phase_shift = 2* M_PI * 90/360;              // Phase shift to start at top of screen

    earth_pos.x = earth_orbit_center.x + cosf(earth_theta + earth_phase_shift) * earth_orbit_radius_x;
    earth_pos.y = earth_orbit_center.y - sinf(earth_theta + earth_phase_shift) * earth_orbit_radius_y;
 
    // Right Key
    if(keyIsPressed(KEY_RIGHT)) {
        // player.vel.x += 250;
        player.vel.x += player_min_vel_x + player_min_max_vel_x * gravity_phase; // Player speed increases with gravity

    }

    // Left Key
    if(keyIsPressed(KEY_LEFT)) {
        // player.vel.x -= 250;
        player.vel.x -= player_min_vel_x + player_min_max_vel_x * gravity_phase;

    }

    // Jump
    if(keyPressedThisFrame(KEY_SPACE) && player.isStanding) {
        // player.vel.y = -750; // Initial setting
        player.vel.y = player_max_vel_y;

        player.state = JUMPING;
        animations[2].start = getTimeInSeconds();
    }

    // Apply gravity
    // player.vel.y += 981 * dt; // Earth Gravity
    player.vel.y += gravity * dt;    // Gravity changes

    // Move Player
    player.pos += player.vel * dt;
    if (player.pos.x < player.size.x / 2) {
        player.pos.x = player.size.x / 2;
    } else if (player.pos.x > level_width - player.size.x / 2) {
        player.pos.x = level_width - player.size.x / 2;
    }

    // Clear standing flag
    player.isStanding = false;

    // Process collisions
    for(int i = 0; i < platforms.size(); i++) {
        collisions(player, platforms[i]);
    }

    // Update Animation
    if(player.state == IDLE) {
        // Idle Transitions
        if(player.isStanding && player.vel.x !=0) {
            player.state = RUNNING;
        } else if(player.isStanding == false) {
            player.state = FALLING;
        }
    } else if(player.state == RUNNING) {
        // Running Transitions
        if(player.isStanding && player.vel.x == 0) {
            player.state = IDLE;
            animations[0].start = getTimeInSeconds();
        } else if(player.isStanding == false) {
            player.state = FALLING;
        }
    } else if(player.state == JUMPING) {
        // Jumping Transitions
        if(player.isStanding && player.vel.x == 0) {
            player.state = IDLE;
        } else if(player.isStanding && player.vel.x != 0) {
            player.state = RUNNING;
        } else if(getTimeInSeconds() > animations[2].start + animations[2].duration) {
            player.state = FALLING;
        }
    } else if(player.state == FALLING) {
        // Falling Transitions
        if(player.isStanding && player.vel.x == 0) {
            player.state = IDLE;
        } else if(player.isStanding && player.vel.x != 0) {
            player.state = RUNNING;
        }
    }
}

// Draw Animation
void drawAnimation(Animation anim, Vec2 pos, Vec2 size) {
    float elapsed = getTimeInSeconds() - anim.start;
    int index = (elapsed / anim.duration) * anim.no_frames;

    if(anim.loop) {
        index = index % anim.no_frames;
    } else {
        index = min(index, anim.no_frames-1);
    }
    drawTexture(anim.frames[index], pos, size);
}

// Render Game
void render(float lag) {
    // White background
    // clear(Color::white);
    clear(gravity_background_colour,gravity_background_colour,gravity_background_colour);

    // Scrolling screen
    Vec2 screen_pos = player.pos;
    screen_pos.x -= screen_scroll_offset;

    // Draw starfield
    for(int i = 0; i < star_count; i++) {
        Vec2 star_pos = Vec2(stars[i].x, stars[i].y);
        // star_pos.x -= screen_scroll_offset * (stars[i].x / level_width); // Stretching starfield 
        
        screen_star_pos = star_pos;
        screen_star_pos.x -= screen_scroll_offset/ (3*(4-stars[i].size));
        
        if (stars[i].star_twinkle_timer == 0) {
            fillEllipse(screen_star_pos, Vec2(stars[i].size, stars[i].size), Color::white);
        }
    }

    // Draw Earth / Sun??
    earth_screen_pos = earth_pos;
    earth_screen_pos.x -= screen_scroll_offset;
    drawTexture(earth_texture, earth_screen_pos, earth_size, 0);



    if(player.state == IDLE) {
        // Play Idle Animation
        drawAnimation(animations[0], screen_pos-player.size/2, player.size);
    } else if(player.state == RUNNING) {
        // Play Running Animation
        if(player.vel.x < 0) {
            Vec2 size = Vec2(-player.size.x, player.size.y);
            size.x *= 376.f/290.f;
            size.y *= 520.f/500.f;
            drawAnimation(animations[1], screen_pos-size/2, size);
        } else {
            Vec2 size = player.size;
            size.x *= 376.f/290.f;
            size.y *= 520.f/500.f;
            drawAnimation(animations[1], screen_pos-size/2, size);
        }
    } else if(player.state == JUMPING) {
        // Play Jumping Animation
        if(player.vel.x < 0) {
            Vec2 size = Vec2(-player.size.x, player.size.y);
            size.x *= 399.f/290.f;
            size.y *= 543.f/500.f;
            drawAnimation(animations[2], screen_pos-size/2, size);
        } else {
            Vec2 size = player.size;
            size.x *= 399.f/290.f;
            size.y *= 543.f/500.f;
            drawAnimation(animations[2], screen_pos-size/2, size);
        }
    } else if(player.state == FALLING) {
        // Play Falling Animation
        if(player.vel.x < 0) {
            Vec2 size = Vec2(-player.size.x, player.size.y);
            size.x *= 399.f/290.f;
            size.y *= 543.f/500.f;
            drawAnimation(animations[3], screen_pos-size/2, size);
        } else {
            Vec2 size = player.size;
            size.x *= 399.f/290.f;
            size.y *= 543.f/500.f;
            drawAnimation(animations[3], screen_pos-size/2, size);
        }
    }

    // Draw Platforms
    for(int i = 0; i < platforms.size(); i++) {
        Vec2 screen_platform_pos = platforms[i].pos;
        screen_platform_pos.x -= screen_scroll_offset;
        fillRect(screen_platform_pos - platforms[i].size/2, platforms[i].size, Color::blue);    
    }
}

// Close the Game
void close() {

}