#include "EnemyBullet.h"
#include "TextureManager.h"
#include "Function.h"
#include <cassert>

void EnemyBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity) {
	// NULLポインタチェック
	assert(model);

	model_ = model;
	// テクスチャの読み込み
	textureHandle_ = TextureManager::Load("gu_Enemy.png");

	worldTransform_.Initialize();
	// 引数で受け取った初期座標をセット
	worldTransform_.translation_ = position;

	// 引数で受け取った速度をメンバ変数に代入
	velocity_ = velocity;
}

void EnemyBullet::Update() {
	worldTransform_.UpdateMatrix();
	// 座標を移動させる（1フレーム分の移動量を足しこむ）
	worldTransform_.translation_ = Add(worldTransform_.translation_, velocity_);
	// 時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}
}

void EnemyBullet::Draw(const ViewProjection& viewProjection) {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, viewProjection, textureHandle_);
}
