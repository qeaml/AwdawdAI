#include "PlayState.hpp"
#include "PlayField.hpp"
#include <nwge/render/window.hpp>

using namespace nwge;

class PlayState: public State {
public:
  bool init() override {
    mField.init();
    return true;
  }

  bool tick(f32 delta) override {
    mField.tick(delta);
    return true;
  }

  void render() const override {
    mField.render();
  }

private:
  PlayField mField;
};

State *getPlayState() {
  return new PlayState;
}
