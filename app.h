#include "maze.h"

#include <SDL2/SDL.h>

#include <map>

class App
{
public:
    App() = default;
    ~App();

    void start();
    void drawLoop();

private:
    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;

    Maze maze_;
    int height_ = 0;
    int width_ = 0;

    bool paused_ = false;
    bool done_ = false;

    std::tuple<bool, unsigned, unsigned> step_{false, 0, 0};
    std::vector<std::pair<unsigned, unsigned>> path_;
    std::vector<SDL_Rect> rects_;
    int animation_step_ = 0;
    std::map<std::pair<unsigned, unsigned>, std::pair<int, SDL_Rect>> bfs_progress_;

    void handleEvents();
    void checkSize();
    void initMaze();
    void drawMaze();
    void drawLightningAnimation();
    void drawProgress();
};