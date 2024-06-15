#include "PlayField.hpp"
#include <nwge/render/draw.hpp>
#include <nwge/render/window.hpp>
#include <random>

using namespace nwge;

void PlayField::init() {
  for(auto &enemy: mEnemies) {
    enemy.randomize();
  }

  static std::mt19937 sEngine{std::random_device()()};
  static std::uniform_real_distribution<f32> sDis{};
  for(auto &tile: mTiles) {
    tile = sDis(sEngine) < 0.25f ? 1 : 0;
  }
}

bool PlayField::lineOfSight(u8 fromX, u8 fromY, u8 toX, u8 toY) const {
  u8 currX = fromX;
  u8 currY = fromY;
  while(currX != toX && currY != toY) {
    u8 stepX0 = currX+1;
    u8 stepY0 = currY;
    u8 stepX1 = currX;
    u8 stepY1 = currY+1;

    s32 dis0 = (stepX0*stepX0 - toX*toX) + (stepY0*stepY0 - toY*toY);
    s32 dis1 = (stepX1*stepX1 - toX*toX) + (stepY1*stepY1 - toY*toY);

    if(dis0 < dis1) {
      currX = stepX0;
      currY = stepY0;
    } else {
      currX = stepX1;
      currY = stepY1;
    }

    if(tile(currX, currY) != 0) {
      return false;
    }
  }
  return true;
}

void PlayField::tick(f32 delta) {
  for(auto &enemy: mEnemies) {
    enemy.tick(delta);
  }

  static std::mt19937 sEngine{std::random_device()()};
  static std::uniform_int_distribution<s32> sDis{-3, 3};

  mDecisionTimer += delta;
  while(mDecisionTimer > cDecisionInterval) {
    mDecisionTimer -= cDecisionInterval;
    s32 chance = sDis(sEngine);
    if(chance == 3 || chance == -3) {
      mEnemies[mDecisionEnemy].decisionOpp(*this, mPlayer);
    }
    if(++mDecisionEnemy == cEnemyCount) {
      mDecisionEnemy = 0;
    }
  }
}

void PlayField::render() const {
  render::clear({1, 1, 1});

  render::color({0, 1, 0});
  f32 tileW = 1.0f / cWidth;
  f32 tileH = 1.0f / cHeight;
  for(s32 y = 0; y < cHeight; ++y) {
  for(s32 x = 0; x < cWidth; ++x) {
    const auto &tileVal = tile(x, y);
    if(tileVal == 0) {
      continue;
    }
    render::rect(
      {x * tileW, y * tileH, cTileZ},
      {tileW, tileH});
  }
  }

  for(const auto &enemy: mEnemies) {
    switch(enemy.state) {
    case Enemy::Stop:
      render::color(cStopStateColor);
      break;
    case Enemy::Wander:
      render::color(cWanderStateColor);
      break;
    case Enemy::Chase:
      render::color(cChaseStateColor);
      break;
    case Enemy::Search:
      render::color(cSearchStateColor);
      break;
    }
    render::rect(
      {enemy.x * tileW, enemy.y * tileH, cEnemyZ},
      {tileW, tileH});
  }

  render::color({0, 0, 1});
  render::rect(
    {mPlayer.x * tileW, mPlayer.y * tileH, cPlayerZ},
    {tileW, tileH});
}

void PlayField::Enemy::randomize() {
  static std::mt19937 sEngine{std::random_device()()};
  static std::uniform_real_distribution<f32> sXDis{0, cWidth};
  static std::uniform_real_distribution<f32> sYDis{0, cHeight};
  x = sXDis(sEngine);
  y = sYDis(sEngine);
}

void PlayField::Enemy::tick(f32 delta) {
  switch(state) {
  case Wander:
  case Chase:
    x += (targetX - x) * cSpeed * delta;
    y += (targetY - y) * cSpeed * delta;
    break;
  case Stop:
  case Search:
    break;
  }
}

void PlayField::Enemy::decisionOpp(const PlayField &field, Player &player) {
  static std::mt19937 sEngine{std::random_device()()};
  static std::uniform_real_distribution<f32> sDis{-3, 3};
  switch(state) {
  case Stop: {
    f32 diffX = x - player.x;
    f32 diffY = y - player.y;
    f32 dis = diffX * diffX - diffY * diffY;
    bool canSee = field.lineOfSight(u8(x), u8(y), u8(player.x), u8(player.y));
    if(dis < cStopSearchRadius && canSee) {
      targetX = player.x;
      targetY = player.y;
      state = Chase;
      break;
    }
    targetX = SDL_clamp(x + sDis(sEngine), 0, cWidth+1);
    targetY = SDL_clamp(y + sDis(sEngine), 0, cHeight+1);
    canSee = field.lineOfSight(u8(x), u8(y), u8(targetX), u8(targetY));
    if(canSee) {
      state = Wander;
    }
    break;
  }
  case Wander: {
    f32 diffX = x - player.x;
    f32 diffY = y - player.y;
    f32 dis = diffX * diffX - diffY * diffY;
    bool canSee = field.lineOfSight(u8(x), u8(y), u8(player.x), u8(player.y));
    if(dis < cStopSearchRadius && canSee) {
      targetX = player.x;
      targetY = player.y;
      state = Chase;
    }
    diffX = x - targetX;
    diffY = y - targetY;
    dis = diffX * diffX - diffY * diffY;
    if(dis < cWanderStopRadius) {
      state = Stop;
    }
    break;
  }
  case Chase: {
    f32 diffX = x - player.x;
    f32 diffY = y - player.y;
    f32 dis = diffX * diffX - diffY * diffY;
    bool canSee = field.lineOfSight(u8(x), u8(y), u8(player.x), u8(player.y));
    if(dis < cChaseSearchRadius && canSee) {
      targetX = player.x;
      targetY = player.y;
    } else {
      state = Search;
    }
    break;
  }
  case Search: {
    f32 diffX = x - player.x;
    f32 diffY = y - player.y;
    f32 dis = diffX * diffX - diffY * diffY;
    bool canSee = field.lineOfSight(u8(x), u8(y), u8(player.x), u8(player.y));
    if(dis < cSearchSearchRadius && canSee) {
      targetX = player.x;
      targetY = player.y;
      state = Chase;
    } else {
      state = Stop;
    }
    break;
  }
  }
}