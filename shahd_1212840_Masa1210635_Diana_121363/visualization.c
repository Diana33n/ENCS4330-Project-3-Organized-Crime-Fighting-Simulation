// File: visualization.c
#include <GL/glut.h>
#include "ipc.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700
#define PI 3.14159265358979323846

// Enhanced color scheme
GLfloat gangColors[10][3] = {
    {0.86, 0.20, 0.18}, {0.13, 0.69, 0.30}, {0.16, 0.32, 0.75},
    {0.95, 0.77, 0.06}, {0.85, 0.11, 0.83}, {0.09, 0.75, 0.81},
    {0.62, 0.42, 0.69}, {0.93, 0.45, 0.02}, {0.00, 0.62, 0.54},
    {0.96, 0.26, 0.21}
};

typedef struct {
    float r, g, b;
} Color;

Color white = {1.0f, 1.0f, 1.0f};
Color yellow = {1.0f, 1.0f, 0.0f};
Color cyan = {0.0f, 1.0f, 1.0f};

// Forward declarations
void renderText(float x, float y, const char *text, Color c);
void drawRoundedRect(float x, float y, float width, float height, float radius);
void drawGlowCircle(float x, float y, float radius, Color c, float intensity);
void drawGradientBackground();
//void drawGangBase(float x, float y, float width, float height, Color c, int in_prison);
void drawHumanFigure(float x, float y, float size, Color c, int rank, int is_agent, int is_caught);
void drawConnection(float x1, float y1, float x2, float y2, float width, Color c);
void drawGangConnections(int gang_id, float center_x, float center_y, float radius);
void drawInfoPanel(float x, float y, float width, float height);
void drawLegend(float x, float y);
void drawGangStatus(float x, float y, int gang_id);
Color getMemberColor(int gang_id, int member_id);
void drawPrisonBars(float x, float y, float width, float height);
void drawBossCrown(float x, float y, float size);
void drawPreparationBar(float x, float y, float width, float height, float progress);
void drawRankBadge(float x, float y, float size, int rank);

void renderText(float x, float y, const char *text, Color c) {
    glColor3f(c.r, c.g, c.b);
    glRasterPos2f(x, y);
    for (const char *c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}
void drawInvestigationArea(float x, float y, int gang_id) {
    glColor4f(0.3f, 0.0f, 0.0f, 0.6f); 
    drawRoundedRect(x, y, 160, 140, 5.0);
    renderText(x + 10, y + 120, "INVESTIGATION", white);

    int drawn = 0;
    for (int i = 0; i < shared_state->gang_member_count[gang_id]; i++) {
        if (shared_state->already_investigated[gang_id][i] &&
            shared_state->member_status[gang_id][i] == 1) {
            
            float mx = x + 20 + (drawn % 2) * 60;
            float my = y + 90 - (drawn / 2) * 40;

            Color c = getMemberColor(gang_id, i);
            drawHumanFigure(mx, my, 10, c,
                            shared_state->member_ranks[gang_id][i], 0,
                            shared_state->catch_agent[gang_id][i]);

            if (shared_state->catch_agent[gang_id][i]) {
                renderText(mx - 15, my - 25, "EXECUTE", (Color){1, 0, 0});
            }

            drawn++;
            if (drawn >= 4) break;
        }
    }
}

void drawRoundedRect(float x, float y, float width, float height, float radius) {
    int segments = 16;
    float angle_step = PI / (2 * segments);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + width/2, y + height/2);

    // Top right corner
    for(int i = 0; i <= segments; i++) {
        float angle = PI/2 - i * angle_step;
        glVertex2f(x + width - radius + cos(angle) * radius,
                   y + height - radius + sin(angle) * radius);
    }

    // Top left corner
    for(int i = 0; i <= segments; i++) {
        float angle = PI - i * angle_step;
        glVertex2f(x + radius + cos(angle) * radius,
                   y + height - radius + sin(angle) * radius);
    }

    // Bottom left corner
    for(int i = 0; i <= segments; i++) {
        float angle = 3*PI/2 - i * angle_step;
        glVertex2f(x + radius + cos(angle) * radius,
                   y + radius + sin(angle) * radius);
    }

    // Bottom right corner
    for(int i = 0; i <= segments; i++) {
        float angle = 2*PI - i * angle_step;
        glVertex2f(x + width - radius + cos(angle) * radius,
                   y + radius + sin(angle) * radius);
    }

    glVertex2f(x + width - radius, y + radius);
    glEnd();
}

void drawGlowCircle(float x, float y, float radius, Color c, float intensity) {
    int segments = 32;
    for(int i = 1; i <= 3; i++) {
        float current_radius = radius + i * 5;
        float alpha = intensity * (1.0 - (float)i/4.0);
        glColor4f(c.r, c.g, c.b, alpha);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for(int j = 0; j <= segments; j++) {
            float angle = 2.0f * PI * (float)j / (float)segments;
            glVertex2f(x + current_radius * cosf(angle),
                       y + current_radius * sinf(angle));
        }
        glEnd();
    }

    glColor3f(c.r, c.g, c.b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for(int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * (float)i / (float)segments;
        glVertex2f(x + radius * cosf(angle), y + radius * sinf(angle));
    }
    glEnd();
}

Color getMemberColor(int gang_id, int member_id) {
    Color c;
    float time_factor = sin(glutGet(GLUT_ELAPSED_TIME) * 0.003f) * 0.5f + 0.5f;

    if (shared_state->catch_agent[gang_id][member_id]) {
        c.r = 1.0f;
        c.g = 0.2f + 0.1f * time_factor;
        c.b = 0.2f + 0.1f * time_factor;
    }
    else if (shared_state->is_under_fake_plan[gang_id][member_id]) {
        c.r = 0.9f + 0.1f * time_factor;
        c.g = 0.7f + 0.2f * time_factor;
        c.b = 0.1f;
    }
    else if (shared_state->knowledge[gang_id][member_id] > 50) {
        float knowledge_factor = shared_state->knowledge[gang_id][member_id] / 100.0f;
        c.r = 0.1f;
        c.g = 0.5f + 0.3f * knowledge_factor * time_factor;
        c.b = 0.9f + 0.1f * knowledge_factor;
    }
    else if (shared_state->suspicion[gang_id][member_id] > 30) {
        float suspicion = shared_state->suspicion[gang_id][member_id] / 100.0f;
        c.r = 0.9f + 0.1f * suspicion;
        c.g = 0.7f - 0.5f * suspicion;
        c.b = 0.1f;
    }
    else {
        float rank_factor = shared_state->member_ranks[gang_id][member_id] / (float)shared_state->num_ranks;
        c.r = gangColors[gang_id][0] * (0.7f + rank_factor * 0.3f);
        c.g = gangColors[gang_id][1] * (0.7f + rank_factor * 0.3f);
        c.b = gangColors[gang_id][2] * (0.7f + rank_factor * 0.3f);
    }

    return c;
}

void drawGradientBackground() {
    glBegin(GL_QUADS);
    glColor3f(0.05f, 0.05f, 0.15f);
    glVertex2f(0, WINDOW_HEIGHT);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glColor3f(0.02f, 0.02f, 0.08f);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(0, 0);
    glEnd();

    glColor4f(0.1f, 0.1f, 0.2f, 0.3f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for(int x = 100; x < WINDOW_WIDTH; x += 100) {
        glVertex2f(x, 0);
        glVertex2f(x, WINDOW_HEIGHT);
    }
    for(int y = 100; y < WINDOW_HEIGHT; y += 100) {
        glVertex2f(0, y);
        glVertex2f(WINDOW_WIDTH, y);
    }
    glEnd();
}

void drawGangBase(float x, float y, float width, float height, int gang_id, int in_prison) {
    // Check if this gang recently succeeded
    int is_success = shared_state->recent_success[gang_id] > 0;
    if (is_success) {
        shared_state->recent_success[gang_id]--; // Decrement counter
    }

    if (in_prison) {
        // Draw prison cell as a solid rectangle with bars
        glColor4f(0.15f, 0.15f, 0.15f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
        glEnd();

        // Draw prison bars
        drawPrisonBars(x, y, width, height);

        // Prison sign
        glColor4f(1.0f, 0.2f, 0.2f, 0.9f);
        //drawRoundedRect(x + width/2 - 20, y + height - 25, 40, 20, 3.0);/////////////////////////////////////////////////////////////////////////
        renderText(x + width/2 - 15, y + height - 15, "PRISON", white);
    } else {
        // Success glow effect
        if (is_success) {
            drawGlowCircle(x + width/2, y + height/2, width*0.7,
                         (Color){0.2f, 0.9f, 0.2f}, 0.7f);

            // Success text
            glColor3f(0.0f, 1.0f, 0.0f);
            renderText(x + width/2 - 30, y + height + 10, "SUCCESS!", (Color){0.0f, 1.0f, 0.0f});
        }

        // Main platform with success color if applicable
       // Color baseColor = is_success ? (Color){0.2f, 0.8f, 0.2f} : c;

        // Shadow
//        glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
//        drawRoundedRect(x + width*0.03, y - height*0.03, width, height*0.1, 5.0);
//
//        // Main platform
//        glColor3f(baseColor.r * 0.7, baseColor.g * 0.7, baseColor.b * 0.7);
//        drawRoundedRect(x, y, width, height*0.1, 5.0);
//
//        // Platform edge highlight
//        glColor3f(baseColor.r * 0.9, baseColor.g * 0.9, baseColor.b * 0.9);
//        glLineWidth(2.0f);
//        glBegin(GL_LINE_LOOP);
//        glVertex2f(x + 5, y + 5);
//        glVertex2f(x + width - 5, y + 5);
//        glVertex2f(x + width - 5, y + height*0.1 - 5);
//        glVertex2f(x + 5, y + height*0.1 - 5);
//        glEnd();
//
//        // Center pillar
//        glBegin(GL_QUADS);
//        glColor3f(baseColor.r * 0.8, baseColor.g * 0.8, baseColor.b * 0.8);
//        glVertex2f(x + width*0.4, y + height*0.1);
//        glVertex2f(x + width*0.6, y + height*0.1);
//        glColor3f(baseColor.r * 0.6, baseColor.g * 0.6, baseColor.b * 0.6);
//        glVertex2f(x + width*0.55, y + height*0.5);
//        glVertex2f(x + width*0.45, y + height*0.5);
//        glEnd();
//
//        // Pillar highlight
//        glColor3f(baseColor.r * 0.9, baseColor.g * 0.9, baseColor.b * 0.9);
//        glLineWidth(1.0f);
//        glBegin(GL_LINES);
//        glVertex2f(x + width*0.45, y + height*0.1);
//        glVertex2f(x + width*0.45, y + height*0.5);
//        glEnd();
//
//        // Top platform
//        glBegin(GL_QUADS);
//        glColor3f(baseColor.r * 0.9, baseColor.g * 0.9, baseColor.b * 0.9);
//        glVertex2f(x + width*0.3, y + height*0.5);
//        glVertex2f(x + width*0.7, y + height*0.5);
//        glColor3f(baseColor.r, baseColor.g, baseColor.b);
//        glVertex2f(x + width*0.65, y + height*0.6);
//        glVertex2f(x + width*0.35, y + height*0.6);
//        glEnd();
//
//        // Platform edge highlight
//        glColor3f(baseColor.r * 1.1, baseColor.g * 1.1, baseColor.b * 1.1);
//        glLineWidth(1.5f);
//        glBegin(GL_LINE_LOOP);
//        glVertex2f(x + width*0.3, y + height*0.5);
//        glVertex2f(x + width*0.7, y + height*0.5);
//        glVertex2f(x + width*0.65, y + height*0.6);
//        glVertex2f(x + width*0.35, y + height*0.6);
//        glEnd();
    }
}


void drawPrisonBars(float x, float y, float width, float height) {
    // Prison bars
    glColor4f(0.5f, 0.5f, 0.5f, 0.8f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);

    // Vertical bars
    for (int i = 1; i <= 5; i++) {
        float bar_x = x + (width * i / 6);
        glVertex2f(bar_x, y + 5);
        glVertex2f(bar_x, y + height - 5);
    }

    // Horizontal bars
    for (int i = 1; i <= 3; i++) {
        float bar_y = y + (height * i / 4);
        glVertex2f(x + 5, bar_y);
        glVertex2f(x + width - 5, bar_y);
    }

    glEnd();

    // Prison border
    glColor4f(0.7f, 0.7f, 0.7f, 0.9f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void drawBossCrown(float x, float y, float size) {
    glColor3f(1.0f, 0.9f, 0.1f); // Gold color

    // Main crown shape
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y + size * 0.8); // Center top
    glVertex2f(x - size * 0.4, y - size * 0.2);
    glVertex2f(x - size * 0.3, y);
    glVertex2f(x - size * 0.2, y - size * 0.1);
    glVertex2f(x, y + size * 0.3);
    glVertex2f(x + size * 0.2, y - size * 0.1);
    glVertex2f(x + size * 0.3, y);
    glVertex2f(x + size * 0.4, y - size * 0.2);
    glEnd();

    // Crown outline
    glColor3f(0.8f, 0.7f, 0.1f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x - size * 0.4, y - size * 0.2);
    glVertex2f(x - size * 0.3, y);
    glVertex2f(x - size * 0.2, y - size * 0.1);
    glVertex2f(x, y + size * 0.3);
    glVertex2f(x + size * 0.2, y - size * 0.1);
    glVertex2f(x + size * 0.3, y);
    glVertex2f(x + size * 0.4, y - size * 0.2);
    glEnd();

    // Jewels
    glColor3f(0.9f, 0.2f, 0.2f); // Ruby
    drawGlowCircle(x, y + size * 0.5, size * 0.1, (Color){0.9f, 0.2f, 0.2f}, 0.5f);

    glColor3f(0.2f, 0.9f, 0.2f); // Emerald
    drawGlowCircle(x - size * 0.25, y + size * 0.2, size * 0.08, (Color){0.2f, 0.9f, 0.2f}, 0.5f);

    glColor3f(0.2f, 0.2f, 0.9f); // Sapphire
    drawGlowCircle(x + size * 0.25, y + size * 0.2, size * 0.08, (Color){0.2f, 0.2f, 0.9f}, 0.5f);
}

void drawRankBadge(float x, float y, float size, int rank) {
    // Draw a badge with the rank number
    glColor3f(0.0f, 0.0f, 0.0f); // Black background
    drawRoundedRect(x  -size/4, y - size/2, size, size, 3.0);

    glColor3f(1.0f, 1.0f, 1.0f); // White text
    char rank_text[4];
    sprintf(rank_text, "%d", rank);

    // Center the text in the badge
    float text_x = x;
    float text_y = y;

    if (rank < 10) {
        text_x -= 1;
    } else {
        text_x -= 3;
    }
    text_y -= 4;

    glRasterPos2f(text_x, text_y);
    for (const char *c = rank_text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

void drawPreparationBar(float x, float y, float width, float height, float progress) {
    // Background
    glColor3f(0.2f, 0.2f, 0.2f);
    drawRoundedRect(x, y, width, height, 2.0);

    // Progress
    glColor3f(0.0f, 1.0f, 0.0f);
    drawRoundedRect(x, y, width * progress, height, 2.0);

    // Border
    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Text showing percentage
    char progress_text[8];
    sprintf(progress_text, "%.0f%%", progress * 100);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x + width/2 - 8, y + height/2 + 4);
    for (const char *c = progress_text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
    }
}

void drawHumanFigure(float x, float y, float size, Color c, int rank, int is_agent, int is_caught) {
    // Lighter shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
    drawGlowCircle(x + size*0.1, y - size*0.1, size*0.8, (Color){0,0,0}, 0.2);

    // Body with more contrast
    glBegin(GL_QUADS);
    glColor3f(c.r, c.g, c.b);
    glVertex2f(x - size*0.25, y + size*0.4);
    glVertex2f(x + size*0.25, y + size*0.4);
    glColor3f(c.r * 0.9, c.g * 0.9, c.b * 0.9);
    glVertex2f(x + size*0.2, y - size*0.3);
    glVertex2f(x - size*0.2, y - size*0.3);
    glEnd();

    // Body outline
    glColor3f(c.r * 0.7, c.g * 0.7, c.b * 0.7);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x - size*0.25, y + size*0.4);
    glVertex2f(x + size*0.25, y + size*0.4);
    glVertex2f(x + size*0.2, y - size*0.3);
    glVertex2f(x - size*0.2, y - size*0.3);
    glEnd();

    // Head with more contrast
    glColor3f(c.r * 1.1, c.g * 1.1, c.b * 1.1);
    glBegin(GL_TRIANGLE_FAN);
    for(int i = 0; i <= 32; i++) {
        float angle = 2.0f * PI * (float)i / 32.0f;
        glVertex2f(x + size*0.35 * cosf(angle), y + size*0.7 + size*0.35 * sinf(angle));
    }
    glEnd();

    // Head outline
    glColor3f(c.r * 0.8, c.g * 0.8, c.b * 0.8);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i <= 16; i++) {
        float angle = 2.0f * PI * (float)i / 16.0f;
        glVertex2f(x + size*0.35 * cosf(angle), y + size*0.7 + size*0.35 * sinf(angle));
    }
    glEnd();

    // Arms
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glColor3f(c.r, c.g, c.b);
    glVertex2f(x - size*0.25, y + size*0.3);
    glVertex2f(x - size*0.4, y + size*0.1);
    glVertex2f(x + size*0.25, y + size*0.3);
    glVertex2f(x + size*0.4, y + size*0.1);
    glEnd();

    // Legs
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glColor3f(c.r, c.g, c.b);
    glVertex2f(x - size*0.1, y - size*0.3);
    glVertex2f(x - size*0.15, y - size*0.5);
    glVertex2f(x + size*0.1, y - size*0.3);
    glVertex2f(x + size*0.15, y - size*0.5);
    glEnd();

    // Draw crown for boss (highest rank)
    if (rank == shared_state->num_ranks-1) {
        drawBossCrown(x, y + size * 0.9, size * 0.6);
    }

    // Agent indicator - more visible
    if (is_agent) {
        float pulse = 0.7f + 0.3f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.005f);
        drawGlowCircle(x, y + size*1.0, size*0.2, (Color){1.0f, 1.0f, 1.0f}, pulse*0.9);

        // Agent "A" symbol
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(x - 3, y + size*1.0 - 5);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'A');
    }

    // Caught agent indicator - more dramatic
    if (is_caught) {
        glColor3f(1.0f, 0.0f, 0.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        // Diagonal cross
        glVertex2f(x - size*0.4, y - size*0.4);
        glVertex2f(x + size*0.4, y + size*0.4);
        glVertex2f(x + size*0.4, y - size*0.4);
        glVertex2f(x - size*0.4, y + size*0.4);
        glEnd();
    }
    if (rank != -1 && rank < shared_state->num_ranks-1) {
        drawRankBadge(x + size*0.4, y + size*0.7, 20, rank);
    }

}

void drawConnection(float x1, float y1, float x2, float y2, float width, Color c) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrt(dx*dx + dy*dy);
    dx /= len;
    dy /= len;

    float px = -dy * width;
    float py = dx * width;

    glBegin(GL_QUADS);
    glColor4f(c.r, c.g, c.b, 0.3f);
    glVertex2f(x1 + px, y1 + py);
    glVertex2f(x1 - px, y1 - py);
    glColor4f(c.r, c.g, c.b, 0.1f);
    glVertex2f(x2 - px, y2 - py);
    glVertex2f(x2 + px, y2 + py);
    glEnd();
}

void drawGangConnections(int gang_id, float center_x, float center_y, float radius) {
    for (int i = 1; i < shared_state->gang_member_count[gang_id]; i++) {
        if (!shared_state->member_status[gang_id][i]) continue;

        float angle1 = 2 * PI * i / shared_state->gang_member_count[gang_id];
        float x1 = center_x + radius * cos(angle1);
        float y1 = center_y + radius * sin(angle1);

        for (int j = i+1; j < shared_state->gang_member_count[gang_id]; j++) {
            if (!shared_state->member_status[gang_id][j]) continue;

            if (abs(i-j) <= 2 || (i == 0 || j == 0)) {
                float angle2 = 2 * PI * j / shared_state->gang_member_count[gang_id];
                float x2 = center_x + radius * cos(angle2);
                float y2 = center_y + radius * sin(angle2);

                Color c = {
                    gangColors[gang_id][0],
                    gangColors[gang_id][1],
                    gangColors[gang_id][2]
                };

                float lineWidth = (i == 0 || j == 0) ? 4.0f : 2.0f;
                drawConnection(x1, y1, x2, y2, lineWidth, c);
            }
        }
    }
}

void drawInfoPanel(float x, float y, float width, float height) {
    // Panel background with subtle gradient
    glBegin(GL_QUADS);
    glColor4f(0.08f, 0.08f, 0.16f, 0.85f);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glColor4f(0.12f, 0.12f, 0.22f, 0.85f);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Border with glow effect
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glColor3f(0.3f, 0.3f, 0.8f);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Title with subtle shadow
    Color shadow = {0.2f, 0.2f, 0.4f};
    renderText(x + 12, y + height - 22, "Gang Activity Monitor", shadow);
    renderText(x + 10, y + height - 20, "Gang Activity Monitor", white);

    // Stats
    char stats[256];
    sprintf(stats, "Successful Plans: %d/%d", shared_state->successful_plans, shared_state->max_successful_plans);
    renderText(x + 15, y + height - 45, stats, white);

    sprintf(stats, "Thwarted Plans: %d/%d", shared_state->thwarted_plans, shared_state->max_thwarted_plans);
    renderText(x + 15, y + height - 65, stats, white);

    sprintf(stats, "Executed Agents: %d/%d", shared_state->executed_agents, shared_state->max_executed_agents);
    renderText(x + 15, y + height - 85, stats, white);

    // Progress bars background
    glColor4f(0.2f, 0.2f, 0.3f, 0.8f);
    drawRoundedRect(x + 160, y + height - 50, 210, 10, 3.0);
    drawRoundedRect(x + 160, y + height - 70, 210, 10, 3.0);
    drawRoundedRect(x + 160, y + height - 90, 210, 10, 3.0);

    // Progress bars
    float progress;

    // Successful plans
    progress = (float)shared_state->successful_plans / shared_state->max_successful_plans;
    glColor3f(0.2f, 0.8f, 0.2f);
    drawRoundedRect(x + 160, y + height - 50, 210 * progress, 10, 3.0);

    // Thwarted plans
    progress = (float)shared_state->thwarted_plans / shared_state->max_thwarted_plans;
    glColor3f(0.8f, 0.2f, 0.2f);
    drawRoundedRect(x + 160, y + height - 70, 210 * progress, 10, 3.0);

    // Executed agents
    progress = (float)shared_state->executed_agents / shared_state->max_executed_agents;
    glColor3f(0.8f, 0.8f, 0.2f);
    drawRoundedRect(x + 160, y + height - 90, 210 * progress, 10, 3.0);
}

void drawLegend(float x, float y) {
    // Panel background with slightly larger size
    glColor4f(0.08f, 0.08f, 0.16f, 0.85f);
    drawRoundedRect(x, y, 220, 240, 5.0);

    // Title with shadow
    Color shadow = {0.2f, 0.2f, 0.4f};
    renderText(x + 12, y + 220, "NETWORK LEGEND:", shadow);
    renderText(x + 10, y + 218, "NETWORK LEGEND:", white);

    // Increased figure size for better visibility
    float fig_size = 14;
    float y_offset = 190;

    // Normal Member
//    drawHumanFigure(x + 15, y + y_offset, fig_size,
//                   (Color){gangColors[0][0], gangColors[0][1], gangColors[0][2]},
//                   -1, 0, 0);
//    renderText(x + 35, y + y_offset - 5, "Normal Member", white);
//    y_offset -= 25;

    // Knows Real Plan (with pulsing effect to match actual visualization)
    float pulse = 0.7f + 0.3f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.005f);
    Color knowledgeable = {0.1f, 0.5f + 0.3f * pulse, 0.9f + 0.1f * pulse};
    drawHumanFigure(x + 15, y + y_offset, fig_size, knowledgeable, -1, 0, 0);
    renderText(x + 35, y + y_offset - 5, "Knows Real Plan", white);
    y_offset -= 25;

    // Suspicious Member
    drawHumanFigure(x + 15, y + y_offset, fig_size,
                   (Color){0.9f, 0.7f, 0.1f}, -1, 0, 0);
    renderText(x + 35, y + y_offset - 5, "Suspicious Member", white);
    y_offset -= 25;

    // Agent
    drawHumanFigure(x + 15, y + y_offset, fig_size,
                   (Color){0.9f, 0.7f, 0.1f}, -1, 1, 0);
    renderText(x + 35, y + y_offset - 5, "Agent (with 'A')", white);
    y_offset -= 25;

    // Under Fake Plan (with pulsing effect)
    Color fakePlan = {0.9f + 0.1f * pulse, 0.7f + 0.2f * pulse, 0.1f};
    drawHumanFigure(x + 15, y + y_offset, fig_size, fakePlan, -1, 0, 0);
    renderText(x + 35, y + y_offset - 5, "Under Fake Plan", white);
    y_offset -= 25;


    // Caught Agent
    drawHumanFigure(x + 15, y + y_offset, fig_size,
                   (Color){1.0f, 0.2f + 0.1f * pulse, 0.2f + 0.1f * pulse},
                   -1, 0, 1);
    renderText(x + 35, y + y_offset - 5, "Caught Agent (X)", white);
    y_offset -= 25;

    // Boss
    int boss_rank = shared_state ? shared_state->num_ranks - 1 : 1;
    char bossText[30];
    sprintf(bossText, "Boss (Rank %d)", boss_rank);
    drawHumanFigure(x + 15, y + y_offset, fig_size,
                   (Color){gangColors[1][0], gangColors[1][1], gangColors[1][2]},
                   boss_rank, 0, 0);
    renderText(x + 35, y + y_offset - 5, bossText, white);
    y_offset -= 25;

    // Prison Status
    glColor4f(0.15f, 0.15f, 0.15f, 0.8f);
    drawRoundedRect(x + 10, y + y_offset - 10, 20, 20, 2.0);
    drawPrisonBars(x + 10, y + y_offset - 10, 20, 20);
    renderText(x + 35, y + y_offset - 5, "Prison Status", white);
    y_offset -= 25;

    // Connections
    Color connColor = {0.5f, 0.5f, 0.8f};
    drawConnection(x + 10, y + y_offset, x + 30, y + y_offset, 2.0f, connColor);
    renderText(x + 35, y + y_offset - 5, "Member Connections", white);
    y_offset -= 25;

    // Rank Indicator
    drawHumanFigure(x + 15, y + y_offset, fig_size,
                   (Color){gangColors[2][0], gangColors[2][1], gangColors[2][2]},
                   -1, 0, 0);
    renderText(x + 35, y + y_offset - 5, "Number = Rank", white);
}

void drawGangStatus(float x, float y, int gang_id) {
    Color gangColor = {
        gangColors[gang_id][0],
        gangColors[gang_id][1],
        gangColors[gang_id][2]
    };

    // Background
    glColor4f(0.1f, 0.1f, 0.2f, 0.7f);
    drawRoundedRect(x, y, 180, 100, 5.0);

    // Title
    char title[50];
    if (shared_state->gang_in_prison[gang_id]) {
        sprintf(title, "Gang %d (PRISON)", gang_id);
        renderText(x + 10, y + 80, title, (Color){1.0f, 0.3f, 0.3f});
    } else {
        sprintf(title, "Gang %d", gang_id);
        renderText(x + 10, y + 80, title, gangColor);
    }

    // Members count
    int alive = 0;
    for(int i = 0; i < shared_state->gang_member_count[gang_id]; i++) {
        if(shared_state->member_status[gang_id][i]) alive++;
    }

    char members[50];
    sprintf(members, "Members: %d/%d", alive, shared_state->gang_member_count[gang_id]);
    renderText(x + 10, y + 60, members, white);

    // Plan status
    if(shared_state->shared_plan_ready[gang_id]) {
        renderText(x + 10, y + 40, "Current Plan:", (Color){0.8f, 0.8f, 0.2f});

        // Display the plan text (truncated if too long)
        char plan_text[60];
        strncpy(plan_text, shared_state->shared_plan[gang_id], 50);
        plan_text[50] = '\0';
        if (strlen(shared_state->shared_plan[gang_id]) > 50) {
            strcat(plan_text, "...");
        }
        renderText(x + 15, y + 20, plan_text, white);
    } else {
        renderText(x + 10, y + 30, "Idle", (Color){0.6f, 0.6f, 0.6f});
    }
}

void drawPoliceStation(float x, float y, float width, float height) {
    // Station base with shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(x + 5, y - 5);
    glVertex2f(x + width + 5, y - 5);
    glVertex2f(x + width + 5, y + height*0.7 - 5);
    glVertex2f(x + 5, y + height*0.7 - 5);
    glEnd();

    // Main building - using regular rectangle instead of rounded
    glColor3f(0.95f, 0.95f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height*0.7);
    glVertex2f(x, y + height*0.7);
    glEnd();

    // Windows
    glColor3f(0.7f, 0.8f, 1.0f);
    for(int i = 0; i < 3; i++) {
        glBegin(GL_QUADS);
        glVertex2f(x + 15 + i*35, y + 20);
        glVertex2f(x + 15 + i*35 + 25, y + 20);
        glVertex2f(x + 15 + i*35 + 25, y + 20 + 30);
        glVertex2f(x + 15 + i*35, y + 20 + 30);
        glEnd();
    }

    // Door
    glColor3f(0.4f, 0.5f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(x + width/2 - 15, y);
    glVertex2f(x + width/2 + 15, y);
    glVertex2f(x + width/2 + 15, y + 40);
    glVertex2f(x + width/2 - 15, y + 40);
    glEnd();

    // Roof
    glColor3f(0.2f, 0.3f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(x, y + height*0.7);
    glVertex2f(x + width, y + height*0.7);
    glVertex2f(x + width - 10, y + height*0.7 + 15);
    glVertex2f(x + 10, y + height*0.7 + 15);
    glEnd();

    // Police sign
    glColor3f(0.1f, 0.1f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(x + width/2 - 25, y + height*0.7 - 10);
    glVertex2f(x + width/2 + 25, y + height*0.7 - 10);
    glVertex2f(x + width/2 + 25, y + height*0.7 + 15);
    glVertex2f(x + width/2 - 25, y + height*0.7 + 15);
    glEnd();
    renderText(x + width/2 - 15, y + height*0.7 + 5, "POLICE", white);

    // Flag pole
    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(x + width - 10, y + height*0.7 + 15);
    glVertex2f(x + width - 10, y + height*0.7 + 40);
    glEnd();

    // Flag
    glColor3f(0.8f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(x + width - 10, y + height*0.7 + 30);
    glVertex2f(x + width - 30, y + height*0.7 + 30);
    glVertex2f(x + width - 30, y + height*0.7 + 40);
    glVertex2f(x + width - 10, y + height*0.7 + 40);
    glEnd();
}
void drawPolicePatrol(float x, float y, int active) {
    // Car body - more modern shape
    glColor3f(0.1f, 0.1f, 0.8f);

    // Main body
    glBegin(GL_POLYGON);
    glVertex2f(x - 25, y - 10);
    glVertex2f(x + 25, y - 10);
    glVertex2f(x + 25, y + 5);
    glVertex2f(x + 15, y + 10);
    glVertex2f(x - 15, y + 10);
    glVertex2f(x - 25, y + 5);
    glEnd();

    // Windshield
    glColor3f(0.7f, 0.8f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(x - 20, y + 5);
    glVertex2f(x + 20, y + 5);
    glVertex2f(x + 12, y + 8);
    glVertex2f(x - 12, y + 8);
    glEnd();

    // Light bar (more realistic)
    float time = glutGet(GLUT_ELAPSED_TIME) * 0.005f;
    if (active) {
        // Flashing lights
        if ((int)time % 2 == 0) {
            // Red light
            glColor3f(1.0f, 0.0f, 0.0f);
            drawRoundedRect(x - 15, y + 10, 10, 5, 2.0);
            // Blue light
            glColor3f(0.0f, 0.0f, 1.0f);
            drawRoundedRect(x + 5, y + 10, 10, 5, 2.0);
        } else {
            // Blue light
            glColor3f(0.0f, 0.0f, 1.0f);
            drawRoundedRect(x - 15, y + 10, 10, 5, 2.0);
            // Red light
            glColor3f(1.0f, 0.0f, 0.0f);
            drawRoundedRect(x + 5, y + 10, 10, 5, 2.0);
        }
    } else {
        // Inactive lights
        glColor3f(0.3f, 0.3f, 0.3f);
        drawRoundedRect(x - 15, y + 10, 30, 5, 2.0);
    }

    // Wheels with more detail
    glColor3f(0.1f, 0.1f, 0.1f);
    drawGlowCircle(x - 15, y - 15, 8, (Color){0,0,0}, 1.0);
    drawGlowCircle(x + 15, y - 15, 8, (Color){0,0,0}, 1.0);

    // Wheel hubs
    glColor3f(0.5f, 0.5f, 0.5f);
    drawGlowCircle(x - 15, y - 15, 4, (Color){0.5f,0.5f,0.5f}, 1.0);
    drawGlowCircle(x + 15, y - 15, 4, (Color){0.5f,0.5f,0.5f}, 1.0);

    // Side stripe
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2f(x - 25, y);
    glVertex2f(x + 25, y);
    glEnd();

    // Police text on side
    renderText(x - 10, y - 5, "POLICE", (Color){1,1,1});
}
void drawPoliceActivity() {
    // Draw police station in top-right
    drawPoliceStation(WINDOW_WIDTH - 180, WINDOW_HEIGHT - 140, 110, 110);

    // Draw patrol cars near gangs in prison
    for (int gang_id = 0; gang_id < shared_state->num_gangs; gang_id++) {
        if (shared_state->gang_in_prison[gang_id]) {
            float gang_x = (WINDOW_WIDTH / (shared_state->num_gangs + 1)) * (gang_id + 1);
            float gang_y = WINDOW_HEIGHT * 0.6f;

            // Position patrol cars near the prison
            drawPolicePatrol(gang_x - 70, gang_y - 150, 1);  // Active patrol
            drawPolicePatrol(gang_x + 70, gang_y - 150, 1);  // Active patrol

            // Draw prison transport
            glColor3f(0.3f, 0.3f, 0.3f);
            drawRoundedRect(gang_x - 50, gang_y - 180, 100, 40, 5.0);
            renderText(gang_x - 30, gang_y - 165, "PRISON TRANSPORT", (Color){1,0,0});
        }
    }

    // Draw police radio waves from station to prisons
    for (int gang_id = 0; gang_id < shared_state->num_gangs; gang_id++) {
        if (shared_state->gang_in_prison[gang_id]) {
            float gang_x = (WINDOW_WIDTH / (shared_state->num_gangs + 1)) * (gang_id + 1);
            float gang_y = WINDOW_HEIGHT * 0.6f;

            float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
            float wave_offset = fmod(time, 1.0f) * 50.0f;

            glColor4f(0.0f, 0.5f, 1.0f, 0.5f);
            glLineWidth(1.5f);
            glLineStipple(2, 0x00FF);
            glEnable(GL_LINE_STIPPLE);
            glBegin(GL_LINES);
            glVertex2f(WINDOW_WIDTH - 120, WINDOW_HEIGHT - 90);
            glVertex2f(gang_x, gang_y - 70 + wave_offset);
            glEnd();
            glDisable(GL_LINE_STIPPLE);
        }
    }
}

void drawMiniProgressBar(float x, float y, float width, float height, float progress) {
    // Background
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Progress - color changes based on completion
    if (progress < 0.5f) {
        glColor3f(1.0f, 0.5f, 0.0f); // Orange for <50%
    } else if (progress < 0.8f) {
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow for 50-80%
    } else {
        glColor3f(0.0f, 1.0f, 0.0f); // Green for >80%
    }

    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width * progress, y);
    glVertex2f(x + width * progress, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Border
    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void display() {
    if (!shared_state) return;

    glClear(GL_COLOR_BUFFER_BIT);

    // Draw gradient background with grid
    drawGradientBackground();

    // Draw gangs in a horizontal arrangement
    float center_y = WINDOW_HEIGHT * 0.62f;
    float gang_spacing = WINDOW_WIDTH / (shared_state->num_gangs + 1);

    for (int gang_id = 0; gang_id < shared_state->num_gangs; gang_id++) {


        // Calculate position horizontally
        float gang_x = gang_spacing * (gang_id + 1);
        float gang_y = center_y;

        // Draw gang base (with prison style if in prison)
        drawGangBase(gang_x - 60, gang_y - 50, 140, 160, gang_id,
                    shared_state->gang_in_prison[gang_id]);

        // Draw members in a circle around the base
        float member_radius = 60.0f;
        for (int member_id = 0; member_id < shared_state->gang_member_count[gang_id]; member_id++) {
            if (!shared_state->member_status[gang_id][member_id]) continue;

            float member_angle = 2 * PI * member_id / shared_state->gang_member_count[gang_id];
            // Inside the member drawing loop in display()
float member_x = gang_x + member_radius * cos(member_angle);
float member_y = gang_y + member_radius * sin(member_angle);

Color memberColor = getMemberColor(gang_id, member_id);
int is_agent = (shared_state->suspicion[gang_id][member_id] > 30);
int is_caught = shared_state->catch_agent[gang_id][member_id];
//int is_under_fake_plan = shared_state->is_under_fake_plan[gang_id][member_id];


// Draw the member
drawHumanFigure(member_x, member_y, 15, memberColor,
              shared_state->member_ranks[gang_id][member_id],
              is_agent, is_caught);

// Calculate preparation progress (0-1)
float prep_progress = shared_state->member_prep_levels[gang_id][member_id] /
                     (float)shared_state->required_prep_level[gang_id];
if (prep_progress > 1.0f) prep_progress = 1.0f;

// Scale the bar width based on required prep level (with min/max limits)
float bar_width = 30.0f; // Base width
float scaled_width = bar_width * (shared_state->required_prep_level[gang_id] / 50.0f);
if (scaled_width < 20.0f) scaled_width = 20.0f; // Minimum width
if (scaled_width > 60.0f) scaled_width = 60.0f; // Maximum width

// Draw preparation bar above the member
if (prep_progress > 0) {
    drawMiniProgressBar(member_x - scaled_width/2,  // Center the bar
                       member_y + 25,
                       scaled_width,
                       5,
                       prep_progress);
}

        }

        // Draw connections between members (not in prison)
        if (!shared_state->gang_in_prison[gang_id]) {
            drawGangConnections(gang_id, gang_x, gang_y, member_radius * 0.7);
        }

        // Draw gang status panel below the gang
        drawGangStatus(gang_x - 90, gang_y - 160, gang_id);
        if (shared_state->investigation_active[gang_id]) {
    drawInvestigationArea(gang_x - 80, gang_y + 160, gang_id);
}

    }

    // Draw info panel
    drawInfoPanel(20, 20, 400, 120);

    // Draw legend
    drawLegend(WINDOW_WIDTH - 220, 20);
    drawPoliceActivity();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
}


void checkTermination() {
    if (shared_state &&
        (shared_state->successful_plans >= shared_state->max_successful_plans ||
         shared_state->thwarted_plans >= shared_state->max_thwarted_plans ||
         shared_state->executed_agents >= shared_state->max_executed_agents)) {
        // More graceful shutdown
        glutDestroyWindow(glutGetWindow());
        sleep(1);
        exit(0);
    }
}

// Then modify the timer function:
void timer() {
    checkTermination();
    glutPostRedisplay();
    glutTimerFunc(1000/30, timer, 0);
}

void initVisualization(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Organized Crime Network Visualization");

    // Enable only necessary features
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Consider disabling these if not absolutely needed
    // glEnable(GL_LINE_SMOOTH);
    // glEnable(GL_POINT_SMOOTH);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Pre-compile display lists here if using them

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(1000/60, timer, 0); // Start with 60 FPS
}

void runVisualization() {
    // Check once before entering the loop
    if (shared_state &&
        (shared_state->successful_plans >= shared_state->max_successful_plans ||
         shared_state->thwarted_plans >= shared_state->max_thwarted_plans ||
         shared_state->executed_agents >= shared_state->max_executed_agents)) {
        return;
    }
    glutMainLoop();
}