#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define WIDTH 800
#define HEIGHT 600
#define PARTICLE_RADIUS 2
#define MAX_PARTICLES 100000
#define GRAVITY 0.3f
#define FLIP_BLEND 0.9f
#define BOUNCE_DAMPING 0.4f
#define COLLISION_DIST 5.0f
#define COLLISION_REPULSE 8.0f
#define FRAME_DELAY_MS 16
#define CELL_SIZE 1
#define GRID_W (WIDTH / CELL_SIZE)
#define GRID_H (HEIGHT / CELL_SIZE)
#define SOLVER_ITERATIONS 6

typedef struct {
    float x, y;
    float vx, vy;
} Particle;

Particle particles[MAX_PARTICLES];
int particle_count = 0;

float grid_vx[HEIGHT][WIDTH];
float grid_vy[HEIGHT][WIDTH];
int grid_count[HEIGHT][WIDTH];

int cursor_x = WIDTH / 2;
int cursor_y = HEIGHT / 2;

int grid_heads[GRID_H][GRID_W];
int grid_next[MAX_PARTICLES];

Uint32 lastTime = 0, currentTime;
int frames = 0;
float fps = 0.0f;

TTF_Font *font = NULL;
SDL_Color fontColor = {255, 255, 255, 255};

void resetSimulation();

void addParticlesAtCursor(int count) {
    for (int i = 0; i < count; i++) {
        if (particle_count >= MAX_PARTICLES) return;
        particles[particle_count].x = cursor_x + (rand() % 5 - 2) * 0.5f;
        particles[particle_count].y = cursor_y + (rand() % 5 - 2) * 0.5f;
        particles[particle_count].vx = (rand() % 100 - 50) / 300.0f;
        particles[particle_count].vy = 0;
        particle_count++;
    }
}

void addInitialParticles() {
    for (int i = 0; i < HEIGHT / 2; i += 4) {
        for (int j = WIDTH / 4; j < 3 * WIDTH / 4; j += 4) {
            if (particle_count >= MAX_PARTICLES) return;
            particles[particle_count].x = j + (rand() % 100 - 50) / 100.0f;
            particles[particle_count].y = i + (rand() % 100 - 50) / 100.0f;
            particles[particle_count].vx = 0;
            particles[particle_count].vy = 0;
            particle_count++;
        }
    }
}

void clearGrid() {
    memset(grid_vx, 0, sizeof(grid_vx));
    memset(grid_vy, 0, sizeof(grid_vy));
    memset(grid_count, 0, sizeof(grid_count));
}

void particlesToGrid() {
    for (int i = 0; i < particle_count; i++) {
        int x = (int)particles[i].x;
        int y = (int)particles[i].y;
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            grid_vx[y][x] += particles[i].vx;
            grid_vy[y][x] += particles[i].vy;
            grid_count[y][x]++;
        }
    }

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (grid_count[y][x] > 0) {
                grid_vx[y][x] /= grid_count[y][x];
                grid_vy[y][x] /= grid_count[y][x];
            }
        }
    }
}

void gridToParticles() {
    for (int i = 0; i < particle_count; i++) {
        int x = (int)particles[i].x;
        int y = (int)particles[i].y;
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            float gvx = grid_vx[y][x];
            float gvy = grid_vy[y][x];
            particles[i].vx = FLIP_BLEND * particles[i].vx + (1.0f - FLIP_BLEND) * gvx;
            particles[i].vy = FLIP_BLEND * particles[i].vy + (1.0f - FLIP_BLEND) * gvy;
        }
    }
}

void buildSpatialGrid() {
    memset(grid_heads, -1, sizeof(grid_heads));
    for (int i = 0; i < particle_count; i++) {
        int cx = (int)(particles[i].x / CELL_SIZE);
        int cy = (int)(particles[i].y / CELL_SIZE);
        if (cx >= 0 && cx < GRID_W && cy >= 0 && cy < GRID_H) {
            grid_next[i] = grid_heads[cy][cx];
            grid_heads[cy][cx] = i;
        } else {
            grid_next[i] = -1;
        }
    }
}

void solveParticleCollisions() {
    for (int iter = 0; iter < SOLVER_ITERATIONS; iter++) {
        buildSpatialGrid();

        for (int i = 0; i < particle_count; i++) {
            Particle *p1 = &particles[i];
            int cx = (int)(p1->x / CELL_SIZE);
            int cy = (int)(p1->y / CELL_SIZE);

            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = cx + dx;
                    int ny = cy + dy;
                    if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) continue;

                    int j = grid_heads[ny][nx];
                    while (j != -1) {
                        if (i >= j) { j = grid_next[j]; continue; }

                        Particle *p2 = &particles[j];
                        float dx = p2->x - p1->x;
                        float dy = p2->y - p1->y;
                        float dist2 = dx * dx + dy * dy;

                        if (dist2 < COLLISION_DIST * COLLISION_DIST && dist2 > 0.0001f) {
                            float dist = sqrtf(dist2);
                            float overlap = (COLLISION_DIST - dist) * 0.5f;
                            float nx = dx / (dist + 1e-6f);
                            float ny = dy / (dist + 1e-6f);

                            // Push particles apart
                            p1->x -= nx * overlap;
                            p1->y -= ny * overlap;
                            p2->x += nx * overlap;
                            p2->y += ny * overlap;
                        }

                        j = grid_next[j];
                    }
                }
            }
        }
    }
}

void moveParticles() {
    for (int i = 0; i < particle_count; i++) {
        particles[i].vy += GRAVITY;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;

        if (particles[i].x < 0) {
            particles[i].x = 0;
            particles[i].vx *= -BOUNCE_DAMPING;
        } else if (particles[i].x > WIDTH - 1) {
            particles[i].x = WIDTH - 1;
            particles[i].vx *= -BOUNCE_DAMPING;
        }

        if (particles[i].y < 0) {
            particles[i].y = 0;
            particles[i].vy *= -BOUNCE_DAMPING;
        } else if (particles[i].y > HEIGHT - 1) {
            particles[i].y = HEIGHT - 1;
            particles[i].vy *= -BOUNCE_DAMPING;
            particles[i].vx *= 0.8f;
        }
    }
}

// Renders text at specified position
void renderText(SDL_Renderer *renderer, const char *text, int x, int y) {
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, fontColor);
    if (!surface) {
        fprintf(stderr, "TTF_RenderText_Blended Error: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    SDL_Rect dstRect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void render(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 10, 10, 40, 255);
    SDL_RenderClear(renderer);

    // Draw water particles
    SDL_SetRenderDrawColor(renderer, 0, 100, 255, 200);
    for (int i = 0; i < particle_count; i++) {
        SDL_Rect r = {
            (int)particles[i].x - PARTICLE_RADIUS,
            (int)particles[i].y - PARTICLE_RADIUS,
            PARTICLE_RADIUS * 2,
            PARTICLE_RADIUS * 2
        };
        SDL_RenderFillRect(renderer, &r);
    }

    // Draw cursor
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_Rect cursor = { cursor_x - 4, cursor_y - 4, 8, 8 };
    SDL_RenderDrawRect(renderer, &cursor);

    // FPS and particle count display
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "FPS: %.2f", fps);
    renderText(renderer, buffer, 10, 10);
    snprintf(buffer, sizeof(buffer), "Particle Count: %d", particle_count);
    renderText(renderer, buffer, 10, 30);

    SDL_RenderPresent(renderer);
}

void handleInput(SDL_Event *e) {
    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode key = e->key.keysym.sym;
        if (key == SDLK_LEFT && cursor_x > 0) cursor_x -= 5;
        if (key == SDLK_RIGHT && cursor_x < WIDTH - 1) cursor_x += 5;
        if (key == SDLK_UP && cursor_y > 0) cursor_y -= 5;
        if (key == SDLK_DOWN && cursor_y < HEIGHT - 1) cursor_y += 5;
        if (key == SDLK_a) addParticlesAtCursor(20);
        if (key == SDLK_r) resetSimulation(); // Reset simulation on 'r'
    }
}

void resetSimulation() {
    particle_count = 0;
    addInitialParticles();
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("FLIP Water Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load font (you can change the path and size)
    font = TTF_OpenFont("arial.ttf", 18);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    resetSimulation();

    bool running = true;
    SDL_Event e;

    lastTime = SDL_GetTicks();
    frames = 0;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
            handleInput(&e);
        }

        clearGrid();
        particlesToGrid();
        gridToParticles();
        buildSpatialGrid();
        solveParticleCollisions();
        moveParticles();

        render(renderer);

        // Calculate FPS every second
        frames++;
        currentTime = SDL_GetTicks();
        if (currentTime - lastTime >= 1000) {
            fps = frames * 1000.0f / (currentTime - lastTime);
            frames = 0;
            lastTime = currentTime;
        }

        SDL_Delay(FRAME_DELAY_MS);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
