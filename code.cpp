#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

struct Button {
    SDL_Rect rect;
    std::string label;
};

SDL_Color white = {255, 255, 255};
SDL_Color gray  = {40, 40, 40};
SDL_Color dark  = {20, 20, 20};

void drawText(SDL_Renderer* r, TTF_Font* f, std::string text, int x, int y) {
    SDL_Surface* surf = TTF_RenderText_Blended(f, text.c_str(), white);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);

    SDL_Rect dst{ x, y, surf->w, surf->h };
    SDL_RenderCopy(r, tex, nullptr, &dst);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

bool inRect(int mx, int my, SDL_Rect rect) {
    return mx >= rect.x && mx <= rect.x + rect.w &&
           my >= rect.y && my <= rect.y + rect.h;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("GUI Calculator",
                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                          300, 450, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    TTF_Font* font = TTF_OpenFont("arial.ttf", 28);

    // Layout buttons
    std::vector<Button> buttons = {
        {{20, 120, 60, 60}, "7"}, {{110, 120, 60, 60}, "8"}, {{200, 120, 60, 60}, "9"},
        {{20, 200, 60, 60}, "4"}, {{110, 200, 60, 60}, "5"}, {{200, 200, 60, 60}, "6"},
        {{20, 280, 60, 60}, "1"}, {{110, 280, 60, 60}, "2"}, {{200, 280, 60, 60}, "3"},
        {{20, 360, 60, 60}, "C"}, {{110, 360, 60, 60}, "0"}, {{200, 360, 60, 60}, "="},

        {{260, 120, 30, 60}, "+"},
        {{260, 200, 30, 60}, "-"},
        {{260, 280, 30, 60}, "*"},
        {{260, 360, 30, 60}, "/"}
    };

    std::string display = "";
    double a = 0, b = 0;
    char op = 0;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mx = event.button.x, my = event.button.y;

                for (auto& btn : buttons) {
                    if (inRect(mx, my, btn.rect)) {
                        std::string label = btn.label;

                        if (label == "C") {
                            display = "";
                            op = 0;
                        }
                        else if (label == "+" || label == "-" || label == "*" || label == "/") {
                            a = std::stod(display);
                            op = label[0];
                            display = "";
                        }
                        else if (label == "=") {
                            b = std::stod(display);
                            double r = 0;

                            switch (op) {
                                case '+': r = a + b; break;
                                case '-': r = a - b; break;
                                case '*': r = a * b; break;
                                case '/': r = (b == 0 ? 0 : a / b); break;
                            }
                            display = std::to_string(r);
                        }
                        else {
                            display += label;
                        }
                    }
                }
            }
        }

        // Draw GUI
        SDL_SetRenderDrawColor(renderer, dark.r, dark.g, dark.b, 255);
        SDL_RenderClear(renderer);

        // Display box
        SDL_Rect displayBox{20, 20, 260, 80};
        SDL_SetRenderDrawColor(renderer, gray.r, gray.g, gray.b, 255);
        SDL_RenderFillRect(renderer, &displayBox);

        drawText(renderer, font, display, 30, 40);

        // Buttons
        for (auto& btn : buttons) {
            SDL_SetRenderDrawColor(renderer, gray.r, gray.g, gray.b, 255);
            SDL_RenderFillRect(renderer, &btn.rect);

            drawText(renderer, font, btn.label,
                     btn.rect.x + 20, btn.rect.y + 15);
        }

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
