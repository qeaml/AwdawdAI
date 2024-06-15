#pragma once

/*
PlayField.hpp
-------------
The field in which you play
*/

#include <nwge/common/def.h>
#include <nwge/glm/glm.hpp>
#include <array>

class PlayField {
public:
  void init();
  void tick(f32 delta);
  void render() const;

private:
  [[nodiscard]]
  constexpr inline u8 tile(u8 x, u8 y) const {
    return mTiles[y * cWidth + x];
  }
  constexpr inline u8 &tile(u8 x, u8 y) {
    return mTiles[y * cWidth + x];
  }

  [[nodiscard]]
  bool lineOfSight(u8 fromX, u8 fromY, u8 toX, u8 toY) const;

  static constexpr s32
    cWidth = 16,
    cHeight = 16,
    cCount = cWidth*cHeight;
  std::array<u8, cCount> mTiles{};

  struct Player {
    f32 x = 1;
    f32 y = 1;
    f32 health = 1;
  } mPlayer;

  static constexpr glm::vec3
    cStopStateColor{0.5, 0, 0},
    cWanderStateColor{1, 0.5, 0.5},
    cChaseStateColor{0, 1, 1},
    cSearchStateColor{0.5, 0.5, 1};

  struct Enemy {
    void randomize();
    f32 x = 0;
    f32 y = 0;
    enum State {
      Stop,   // Enemy stopped, not doing anything
      Wander, // Enemy wandering to some random place
      Chase,  // Enemy chasing after player
      Search, // Enemy searching for player after chase
    } state = State::Stop;
    static constexpr f32
      cStopSearchRadius = 2.1f,
      cWanderStopRadius = 0.5f,
      cChaseSearchRadius = 3.3f,
      cSearchSearchRadius = 4.9f,
      cSpeed = 0.5f;
    f32 targetX = 0;
    f32 targetY = 0;
    void tick(f32 delta);
    void decisionOpp(const PlayField &field, Player &player);
  };

  static constexpr s32
    cEnemyCount = 8;
  std::array<Enemy, cEnemyCount> mEnemies{};

  static constexpr f32
    cDecisionInterval = 1.0f / cEnemyCount;
  s32 mDecisionEnemy = 0;
  f32 mDecisionTimer = 0.0f;

  #define NEXT_Z (0.9f-__COUNTER__*0.01f)

  static constexpr f32
    cTileZ = NEXT_Z,
    cEnemyZ = NEXT_Z,
    cPlayerZ = NEXT_Z;

  #undef NEXT_Z
};