#pragma once
#include <vector>
#include <memory>
#include <functional>

class Drawable;
class Camera;

enum class QueueType {
    Opaque,      // Front-to-back after material sorting
    Transparent, // Back-to-front after material sorting
    Shadow       // Material batching prioritized for shadow passes
};

class RenderQueue {
public:
    explicit RenderQueue(QueueType type);
    
    // Queue management
    void clear();
    void add(std::shared_ptr<Drawable> drawable);
    void sort(const Camera& camera);
    
    // Access
    const std::vector<std::shared_ptr<Drawable>>& getDrawables() const { return _drawables; }
    size_t size() const { return _drawables.size(); }
    bool empty() const { return _drawables.empty(); }
    
    // Iterator support
    auto begin() const { return _drawables.begin(); }
    auto end() const { return _drawables.end(); }

private:
    QueueType _type;
    std::vector<std::shared_ptr<Drawable>> _drawables;
    
    // Sorting functions
    bool compareOpaque(const std::shared_ptr<Drawable>& a, 
                      const std::shared_ptr<Drawable>& b, 
                      const Camera& camera) const;
    bool compareTransparent(const std::shared_ptr<Drawable>& a, 
                           const std::shared_ptr<Drawable>& b, 
                           const Camera& camera) const;
    bool compareShadow(const std::shared_ptr<Drawable>& a, 
                      const std::shared_ptr<Drawable>& b, 
                      const Camera& camera) const;
};
