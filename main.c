// main.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <termios.h>
#include <fcntl.h>

#define WIDTH 50
#define HEIGHT 25
#define FRAME_DELAY 45000
#define MAX_PARTICLES 100000
#define GRAVITY 0.15f
#define FLIP_BLEND 0.7f
#define BOUNCE_DAMPING 0.8f
#define COLLISION_DIST 0.7f
#define COLLISION_REPULSE 1.0f

typedef struct {
    float x, y;
    float vx, vy;
} Particle;

Particle particles[MAX_PARTICLES];
int particle_count = 0;

float grid_vx[HEIGHT + 1][WIDTH + 1];
float grid_vy[HEIGHT + 1][WIDTH + 1];
int grid_count[HEIGHT + 1][WIDTH + 1];

char display[HEIGHT][WIDTH];

int cursor_x = WIDTH / 2;
int cursor_y = HEIGHT / 2;

void setNonBlockingInput(int enable) {
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    if (enable) {
        ttystate.c_lflag &= ~(ICANON | ECHO);
        ttystate.c_cc[VMIN] = 0;
        ttystate.c_cc[VTIME] = 0;
    } else {
        ttystate.c_lflag |= ICANON | ECHO;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
    fcntl(STDIN_FILENO, F_SETFL, enable ? O_NONBLOCK : 0);
}

void handleInput() {
    char buf[3];
    int n = read(STDIN_FILENO, buf, sizeof(buf));
    if (n <= 0) return;
    if (buf[0] == 'q') exit(0);
    if (buf[0] == 'r' || buf[0] == 'R') {
        particle_count = 0;
        for (int i = 0; i < 100; i++) {
            particles[particle_count].x = WIDTH / 2 + (rand() % 5 - 2);
            particles[particle_count].y = (float)i * (HEIGHT / 100.0f); // same vertical spread
            particles[particle_count].vx = (rand() % 100 - 50) / 500.0f;
            particles[particle_count].vy = 0;
            particle_count++;
        }
    }

    if (buf[0] == 'a') {
        for (int i = 0; i < 10; i++) {
            if (particle_count < MAX_PARTICLES) {
                particles[particle_count].x = cursor_x + (rand() % 5 - 2) * 0.1f;
                particles[particle_count].y = cursor_y + (rand() % 5 - 2) * 0.1f;
                particles[particle_count].vx = (rand() % 100 - 50) / 500.0f;
                particles[particle_count].vy = 0;
                particle_count++;
            }
        }
    } else if (buf[0] == '\033' && n >= 3) {
        if (buf[1] == '[') {
            if (buf[2] == 'A' && cursor_y > 0) cursor_y--;
            else if (buf[2] == 'B' && cursor_y < HEIGHT - 1) cursor_y++;
            else if (buf[2] == 'C' && cursor_x < WIDTH - 1) cursor_x++;
            else if (buf[2] == 'D' && cursor_x > 0) cursor_x--;
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

void moveParticles() {
    for (int i = 0; i < particle_count; i++) {
        particles[i].vy += GRAVITY;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;

        for (int j = i + 1; j < particle_count; j++) {
            float dx = particles[j].x - particles[i].x;
            float dy = particles[j].y - particles[i].y;
            float dist2 = dx*dx + dy*dy;
            if (dist2 > 0 && dist2 < COLLISION_DIST*COLLISION_DIST) {
                float dist = sqrtf(dist2);
                float overlap = COLLISION_DIST - dist;
                float nx = dx / (dist + 1e-6f);
                float ny = dy / (dist + 1e-6f);
                particles[i].x -= nx * overlap * 0.5f;
                particles[i].y -= ny * overlap * 0.5f;
                particles[j].x += nx * overlap * 0.5f;
                particles[j].y += ny * overlap * 0.5f;
                float relvx = particles[j].vx - particles[i].vx;
                float relvy = particles[j].vy - particles[i].vy;
                float sepVel = relvx * nx + relvy * ny;
                if (sepVel < 0) {
                    float impulse = -sepVel * 0.5f;
                    particles[i].vx -= impulse * nx;
                    particles[i].vy -= impulse * ny;
                    particles[j].vx += impulse * nx;
                    particles[j].vy += impulse * ny;
                }
            }
        }

        if (particles[i].x < 0) {
            particles[i].x = 0;
            particles[i].vx *= -BOUNCE_DAMPING;
        } else if (particles[i].x >= WIDTH - 1) {
            particles[i].x = WIDTH - 1;
            particles[i].vx *= -BOUNCE_DAMPING;
        }
        if (particles[i].y < 0) {
            particles[i].y = 0;
            particles[i].vy *= -BOUNCE_DAMPING;
        } else if (particles[i].y >= HEIGHT - 1) {
            particles[i].y = HEIGHT - 1;
            particles[i].vy *= -BOUNCE_DAMPING;
            particles[i].vx *= 0.8f;
        }
    }
}

void drawDisplay() {
    printf("\033[H");
    memset(display, '.', sizeof(display));
    for (int i = 0; i < particle_count; i++) {
        int x = (int)particles[i].x;
        int y = (int)particles[i].y;
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            display[y][x] = 'o';
        }
    }
    display[cursor_y][cursor_x] = '+';
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            putchar(display[y][x]);
        }
        putchar('\n');
    }
    fflush(stdout);
}

int main() {
    printf("\033[?25l");
    printf("\033[2J");

    setNonBlockingInput(1);

    for (int i = 0; i < 100; i++) {
        particles[particle_count].x = WIDTH / 2 + (rand() % 5 - 2);
        particles[particle_count].y = (float)i * (HEIGHT / 100.0f);
        particles[particle_count].vx = (rand() % 100 - 50) / 500.0f;
        particles[particle_count].vy = 0;
        particle_count++;
    }


    while (1) {
        handleInput();
        clearGrid();
        particlesToGrid();
        gridToParticles();
        moveParticles();
        drawDisplay();
        usleep(FRAME_DELAY);
    }

    setNonBlockingInput(0);
    printf("\033[?25h");
    return 0;
}
