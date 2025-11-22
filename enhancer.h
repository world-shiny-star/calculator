// calculator_features.h
// Advanced features add-on for your black-themed calculator
// Adds: History, Backspace, Percentage, Scientific functions, Keyboard support

#ifndef CALCULATOR_FEATURES_H
#define CALCULATOR_FEATURES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <cmath>
#include <deque>

// ============================================================================
// FEATURE 1: CALCULATION HISTORY
// ============================================================================
struct HistoryEntry {
    std::string expression;
    std::string result;
};

class CalculatorHistory {
private:
    std::deque<HistoryEntry> history;
    const size_t maxHistory = 10;
    
public:
    void addEntry(const std::string& expr, const std::string& result) {
        history.push_front({expr, result});
        if (history.size() > maxHistory) {
            history.pop_back();
        }
    }
    
    void clear() {
        history.clear();
    }
    
    const std::deque<HistoryEntry>& getHistory() const {
        return history;
    }
    
    // Draw history sidebar
    void draw(SDL_Renderer* renderer, TTF_Font* font, int windowWidth, int windowHeight) {
        if (history.empty()) return;
        
        int sidebarWidth = 200;
        int sidebarX = windowWidth - sidebarWidth;
        
        // Semi-transparent background
        SDL_SetRenderDrawColor(renderer, 15, 15, 15, 230);
        SDL_Rect sidebar = {sidebarX, 0, sidebarWidth, windowHeight};
        SDL_RenderFillRect(renderer, &sidebar);
        
        // Title
        SDL_Color titleColor = {150, 150, 150, 255};
        renderTextSimple(renderer, font, "History", sidebarX + 10, 10, titleColor);
        
        // Entries
        int y = 50;
        SDL_Color entryColor = {200, 200, 200, 255};
        SDL_Color resultColor = {100, 200, 100, 255};
        
        for (const auto& entry : history) {
            if (y > windowHeight - 50) break;
            
            renderTextSimple(renderer, font, entry.expression, sidebarX + 10, y, entryColor);
            y += 25;
            renderTextSimple(renderer, font, "= " + entry.result, sidebarX + 10, y, resultColor);
            y += 35;
        }
    }
    
private:
    void renderTextSimple(SDL_Renderer* r, TTF_Font* f, const std::string& text, int x, int y, SDL_Color color) {
        SDL_Surface* surf = TTF_RenderText_Blended(f, text.c_str(), color);
        if (!surf) return;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
        SDL_Rect dst{x, y, surf->w, surf->h};
        SDL_RenderCopy(r, tex, nullptr, &dst);
        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
    }
};

// ============================================================================
// FEATURE 2: KEYBOARD SUPPORT
// ============================================================================
class KeyboardHandler {
public:
    static char getKeyChar(SDL_Event& event) {
        if (event.type != SDL_KEYDOWN) return 0;
        
        SDL_Keycode key = event.key.keysym.sym;
        
        // Numbers
        if (key >= SDLK_0 && key <= SDLK_9) return '0' + (key - SDLK_0);
        if (key >= SDLK_KP_0 && key <= SDLK_KP_9) return '0' + (key - SDLK_KP_0);
        
        // Operators
        if (key == SDLK_PLUS || key == SDLK_KP_PLUS) return '+';
        if (key == SDLK_MINUS || key == SDLK_KP_MINUS) return '-';
        if (key == SDLK_ASTERISK || key == SDLK_KP_MULTIPLY) return '*';
        if (key == SDLK_SLASH || key == SDLK_KP_DIVIDE) return '/';
        if (key == SDLK_PERIOD || key == SDLK_KP_PERIOD) return '.';
        
        // Special keys
        if (key == SDLK_RETURN || key == SDLK_KP_ENTER) return '=';
        if (key == SDLK_ESCAPE || key == SDLK_c) return 'C';
        if (key == SDLK_BACKSPACE) return 'B'; // Backspace
        if (key == SDLK_PERCENT) return '%';
        
        return 0;
    }
};

// ============================================================================
// FEATURE 3: BACKSPACE FUNCTION
// ============================================================================
inline void backspaceInput(std::string& display) {
    if (display.length() > 1) {
        display.pop_back();
    } else {
        display = "0";
    }
}

// ============================================================================
// FEATURE 4: PERCENTAGE CALCULATION
// ============================================================================
inline double calculatePercentage(double value, double percentage) {
    return value * (percentage / 100.0);
}

// ============================================================================
// FEATURE 5: SCIENTIFIC FUNCTIONS
// ============================================================================
class ScientificCalculator {
public:
    static double sqrt(double x) { return std::sqrt(x); }
    static double square(double x) { return x * x; }
    static double inverse(double x) { return (x != 0) ? 1.0 / x : 0; }
    static double sin(double x) { return std::sin(x * M_PI / 180.0); }
    static double cos(double x) { return std::cos(x * M_PI / 180.0); }
    static double tan(double x) { return std::tan(x * M_PI / 180.0); }
    static double log(double x) { return (x > 0) ? std::log10(x) : 0; }
    static double ln(double x) { return (x > 0) ? std::log(x) : 0; }
    static double power(double base, double exp) { return std::pow(base, exp); }
};

// ============================================================================
// FEATURE 6: MEMORY FUNCTIONS
// ============================================================================
class CalculatorMemory {
private:
    double memory = 0.0;
    
public:
    void clear() { memory = 0.0; }
    void add(double value) { memory += value; }
    void subtract(double value) { memory -= value; }
    void store(double value) { memory = value; }
    double recall() const { return memory; }
    bool hasMemory() const { return memory != 0.0; }
};

// ============================================================================
// FEATURE 7: BUTTON ANIMATION/FEEDBACK
// ============================================================================
class ButtonAnimator {
private:
    SDL_Rect animRect = {0, 0, 0, 0};
    Uint32 animStartTime = 0;
    const Uint32 animDuration = 100; // ms
    
public:
    void triggerAnimation(SDL_Rect rect) {
        animRect = rect;
        animStartTime = SDL_GetTicks();
    }
    
    void draw(SDL_Renderer* renderer) {
        Uint32 elapsed = SDL_GetTicks() - animStartTime;
        if (elapsed > animDuration) return;
        
        float progress = (float)elapsed / animDuration;
        int alpha = (int)(255 * (1.0f - progress));
        
        SDL_SetRenderDrawColor(renderer, 100, 150, 255, alpha);
        SDL_RenderFillRect(renderer, &animRect);
    }
    
    bool isAnimating() const {
        return (SDL_GetTicks() - animStartTime) <= animDuration;
    }
};

// ============================================================================
// FEATURE 8: ERROR HANDLING
// ============================================================================
inline bool isValidNumber(const std::string& str) {
    if (str.empty() || str == "Error" || str == "Infinity") return false;
    try {
        std::stod(str);
        return true;
    } catch (...) {
        return false;
    }
}

inline std::string safeCalculate(double a, double b, char op) {
    double result = 0;
    
    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/':
            if (b == 0) return "Error: Div by 0";
            result = a / b;
            break;
        default: return "Error";
    }
    
    if (std::isnan(result) || std::isinf(result)) {
        return "Error";
    }
    
    return std::to_string(result);
}

// ============================================================================
// FEATURE 9: SOUND EFFECTS (Optional - requires SDL_mixer)
// ============================================================================
#ifdef USE_SOUND_EFFECTS
#include <SDL2/SDL_mixer.h>

class SoundEffects {
private:
    Mix_Chunk* clickSound = nullptr;
    Mix_Chunk* errorSound = nullptr;
    
public:
    bool init() {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            return false;
        }
        // Load your sound files here
        // clickSound = Mix_LoadWAV("click.wav");
        return true;
    }
    
    void playClick() {
        if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
    }
    
    void playError() {
        if (errorSound) Mix_PlayChannel(-1, errorSound, 0);
    }
    
    ~SoundEffects() {
        if (clickSound) Mix_FreeChunk(clickSound);
        if (errorSound) Mix_FreeChunk(errorSound);
        Mix_CloseAudio();
    }       
};  
#endif

// ============================================================================
// FEATURE 10: COPY/PASTE SUPPORT
// ============================================================================
inline void copyToClipboard(const std::string& text) {
    SDL_SetClipboardText(text.c_str());
}

inline std::string pasteFromClipboard() {
    char* text = SDL_GetClipboardText();
    std::string result = text ? text : "";
    SDL_free(text);
    return result;
}

#endif 