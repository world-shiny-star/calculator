#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> // Required for rendering text
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <iomanip>

// --- Configuration Constants ---
const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 550; // Increased height for better layout
const int BUTTON_WIDTH = 90;
const int BUTTON_HEIGHT = 70;
const int PADDING = 8;

// --- Colors (R, G, B, A) ---
const SDL_Color COLOR_BG = {10, 10, 10, 255};      // Dark Window Background
const SDL_Color COLOR_BUTTON = {30, 30, 30, 255};  // Button Background
const SDL_Color COLOR_HOVER = {50, 50, 50, 255};   // Button Hover
const SDL_Color COLOR_TEXT = {255, 255, 255, 255}; // White Text
const SDL_Color COLOR_DISPLAY_BG = {20, 20, 20, 255}; // Display Background

// --- Global State ---
SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
TTF_Font* g_font = nullptr;

std::string input_str = "0";
double storedValue = 0.0;
char currentOp = 0;
bool resetInput = true; // Start ready for input

// --- Button Structure ---
struct Button {
    SDL_Rect rect;
    std::string label;
    std::string type; // "number", "operator", "clear", "equals"
    char value;

    bool is_hovered(int mouse_x, int mouse_y) const {
        return mouse_x >= rect.x && mouse_x <= rect.x + rect.w &&
               mouse_y >= rect.y && mouse_y <= rect.y + rect.h;
    }
};

std::vector<Button> buttons;

// --- Forward Declarations ---
bool init_sdl();
void cleanup();
void render_text(const std::string& text, int x, int y, SDL_Color color);
void layout_buttons();
void handle_button_press(const Button& btn);
std::string format_double(double val);
void calculate_result();

// --- Calculator Logic ---

std::string format_double(double val) {
    // Custom formatting to resemble '%g' (no trailing zeros for integers)
    std::stringstream ss;
    ss << std::setprecision(10); // Set high precision
    if (std::abs(val - std::round(val)) < 1e-9) {
        ss << std::fixed << std::setprecision(0) << val;
    } else {
        ss << val;
    }
    return ss.str();
}

void calculate_result() {
    if (currentOp == 0) return;

    double a = storedValue;
    double b = 0.0;
    try {
        b = std::stod(input_str);
    } catch (...) {
        b = 0.0;
    }

    double result = 0;
    bool error = false;

    switch (currentOp) {
    case '+': result = a + b; break;
    case '-': result = a - b; break;
    case '*': result = a * b; break;
    case '/':
        if (b != 0) {
            result = a / b;
        } else {
            input_str = "Error";
            error = true;
        }
        break;
    }

    if (!error) {
        input_str = format_double(result);
        storedValue = result;
    } else {
        storedValue = 0.0;
    }
    currentOp = 0;
    resetInput = true;
}

void handle_button_press(const Button& btn) {
    if (btn.type == "number") {
        if (resetInput) {
            input_str = (btn.value == '.') ? "0." : btn.label;
            resetInput = false;
        } else {
            if (btn.value == '.' && input_str.find('.') != std::string::npos) {
                // Ignore multiple decimals
                return;
            }
            if (input_str == "0" && btn.value != '.') {
                input_str = btn.label; // Replace initial zero
            } else {
                input_str += btn.label;
            }
        }
    } else if (btn.type == "operator") {
        if (!resetInput) { // If there's a pending calculation, run it first
            if (currentOp != 0) {
                calculate_result();
                storedValue = std::stod(input_str);
            } else {
                 try { storedValue = std::stod(input_str); } catch (...) { storedValue = 0.0; }
            }
        }
        currentOp = btn.value;
        resetInput = true;
    } else if (btn.type == "clear") {
        input_str = "0";
        storedValue = 0.0;
        currentOp = 0;
        resetInput = true;
    } else if (btn.type == "equals") {
        calculate_result();
    }
}

// --- SDL Graphics and Setup ---

// Renders text using the loaded font
void render_text(const std::string& text, int x, int y, SDL_Color color) {
    if (text.empty() || !g_renderer || !g_font) return;

    // Create a surface from the text
    SDL_Surface* surface = TTF_RenderText_Solid(g_font, text.c_str(), color);
    if (!surface) {
        fprintf(stderr, "TTF_RenderText_Solid error: %s\n", TTF_GetError());
        return;
    }

    // Create a texture from the surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, surface);
    if (!texture) {
        fprintf(stderr, "SDL_CreateTextureFromSurface error: %s\n", SDL_GetError());
    }

    // Determine position and size
    SDL_Rect dstrect = {x, y, surface->w, surface->h};

    // Render and clean up
    if (texture) {
        SDL_RenderCopy(g_renderer, texture, NULL, &dstrect);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void layout_buttons() {
    // Array of button labels
    const std::vector<std::pair<std::string, std::string>> labels = {
        {"7", "number"}, {"8", "number"}, {"9", "number"}, {"/", "operator"},
        {"4", "number"}, {"5", "number"}, {"6", "number"}, {"*", "operator"},
        {"1", "number"}, {"2", "number"}, {"3", "number"}, {"-", "operator"},
        {".", "number"}, {"0", "number"}, {"C", "clear"},   {"+", "operator"},
        {"=", "equals"}
    };

    int current_x = PADDING;
    int current_y = 100 + PADDING; // Start below display area

    for (size_t i = 0; i < labels.size(); ++i) {
        Button btn;
        btn.label = labels[i].first;
        btn.type = labels[i].second;
        btn.value = btn.label[0]; // Store char value

        if (btn.type == "equals") {
            btn.rect = {current_x, current_y, WINDOW_WIDTH - (2 * PADDING), BUTTON_HEIGHT};
        } else {
            btn.rect = {current_x, current_y, BUTTON_WIDTH, BUTTON_HEIGHT};
        }

        buttons.push_back(btn);

        // Move cursor for next button
        if (btn.type != "equals") {
            current_x += BUTTON_WIDTH + PADDING;
            if ((i + 1) % 4 == 0) { // New row after every 4 buttons
                current_x = PADDING;
                current_y += BUTTON_HEIGHT + PADDING;
            }
        } else {
            // After the equals button, we are done
            break;
        }
    }
}

bool init_sdl() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return false;
    }

    g_window = SDL_CreateWindow("Vanilla SDL Calculator",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                WINDOW_WIDTH, WINDOW_HEIGHT,
                                SDL_WINDOW_SHOWN);
    if (!g_window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        cleanup();
        return false;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        cleanup();
        return false;
    }

    // Load a font (You need to provide a path to a .ttf file, e.g., Arial.ttf)
    // Replace "path/to/font.ttf" with a valid font file path on your system
    g_font = TTF_OpenFont("arial.ttf", 36); 
    if (!g_font) {
        fprintf(stderr, "TTF_OpenFont failed: %s. Using default font.\n", TTF_GetError());
    }
    
    // Set up button layout
    layout_buttons();

    return true;
}

void cleanup() {
    if (g_font) TTF_CloseFont(g_font);
    if (g_renderer) SDL_DestroyRenderer(g_renderer);
    if (g_window) SDL_DestroyWindow(g_window);
    TTF_Quit();
    SDL_Quit();
}

// --- Main Loop and Rendering ---

int main(int argc, char* argv[]) {
    if (!init_sdl()) {
        return 1;
    }

    bool quit = false;
    SDL_Event event;

    while (!quit) {
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                for (const auto& btn : buttons) {
                    if (btn.is_hovered(event.button.x, event.button.y)) {
                        handle_button_press(btn);
                        break;
                    }
                }
            }
        }

        // --- Drawing ---
        SDL_SetRenderDrawColor(g_renderer, COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, 255);
        SDL_RenderClear(g_renderer);

        // 1. Draw Display Background
        SDL_Rect display_rect = {PADDING, PADDING, WINDOW_WIDTH - (2 * PADDING), 80};
        SDL_SetRenderDrawColor(g_renderer, COLOR_DISPLAY_BG.r, COLOR_DISPLAY_BG.g, COLOR_DISPLAY_BG.b, 255);
        SDL_RenderFillRect(g_renderer, &display_rect);
        
        // 2. Render Input Text (right-aligned)
        int text_width = 0, text_height = 0;
        // Estimate text size for right alignment (pure SDL requires extra steps, TTF_SizeText helps)
        TTF_SizeText(g_font, input_str.c_str(), &text_width, &text_height);
        
        // Position: X = Display Right Edge - Text Width - PADDING, Y = Display Top + PADDING
        int text_x = display_rect.x + display_rect.w - text_width - PADDING;
        int text_y = display_rect.y + (display_rect.h / 2) - (text_height / 2); // Center vertically
        
        render_text(input_str, text_x, text_y, COLOR_TEXT);

        // 3. Draw Buttons
        for (const auto& btn : buttons) {
            SDL_Color color = btn.is_hovered(mouse_x, mouse_y) ? COLOR_HOVER : COLOR_BUTTON;
            
            SDL_SetRenderDrawColor(g_renderer, color.r, color.g, color.b, 255);
            SDL_RenderFillRect(g_renderer, &btn.rect);
            
            // Draw button text (centered)
            TTF_SizeText(g_font, btn.label.c_str(), &text_width, &text_height);
            int btn_text_x = btn.rect.x + (btn.rect.w / 2) - (text_width / 2);
            int btn_text_y = btn.rect.y + (btn.rect.h / 2) - (text_height / 2);
            render_text(btn.label, btn_text_x, btn_text_y, COLOR_TEXT);
        }

        // Show the drawn frame
        SDL_RenderPresent(g_renderer);
    }

    cleanup();
    return 0;
}

