#include "Enemy.h"
#include "Function.h"
#include "imgui.h"
#include <cassert>

Enemy::~Enemy() {
	for (EnemyBullet* bullet : bullets_) {
		delete bullet;
	}
}

void Enemy::Initialize(Model* model, uint32_t textureHandle) {
	// NULLポインタチェック
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;

	worldTransform_.Initialize();

	worldTransform_.translation_ = {10, 2, 30};

	InitializeApproach();
}

void Enemy::Update() {

	/*switch (phase_) {
	case Phase::Approach:
	default:
		Approach();
		break;

	case Phase::Leave:
		Leave();
		break;
	}*/

	(this->*phaseTable[static_cast<size_t>(phase_)])();

	// 弾更新
	for (EnemyBullet* bullet : bullets_) {
		bullet->Update();
	}

	worldTransform_.UpdateMatrix();

	ImGui::Begin("enemy");

	ImGui::DragFloat3("enemy_pos", &worldTransform_.translation_.x, 0.1f);

	ImGui::End();
}

void Enemy::Draw(const ViewProjection& viewProjection) {

	// 3Dモデルを描画
	model_->Draw(worldTransform_, viewProjection, textureHandle_);

	// 弾描画
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw(viewProjection);
	}
}

void Enemy::Approach() {
	// キャラクターの移動ベクトル
	Vector3 move = {0, 0, 0};

	// 移動速度
	const float kCharacterSpeed = 0.1f;

	move.z -= kCharacterSpeed;

	// 移動（ベクトルを加算）
	worldTransform_.translation_ = Add(worldTransform_.translation_, move);
	// 規定の位置に到達したら離脱
	if (worldTransform_.translation_.z < 0.0f) {
		phase_ = Phase::Leave;
	}

	// 発射タイマーをカウントダウン
	fireTimer_--;
	// 指定時間に達した
	if (fireTimer_ <= 0) {
		// 弾を発射
		Fire();
		// 発射タイマーの初期化
		fireTimer_ = kFireInterval;
	}
}

void Enemy::InitializeApproach() {
	// 発射タイマーを初期化
	fireTimer_ = kFireInterval;
}

//void Enemy::InitializeLeave() {}

void Enemy::Leave() {

	// キャラクターの移動ベクトル
	Vector3 move = {0, 0, 0};

	// 移動速度
	const float kCharacterSpeed = 0.01f;

	move.x -= kCharacterSpeed;
	move.y += kCharacterSpeed;

	// 移動（ベクトルを加算）
	worldTransform_.translation_ = Add(worldTransform_.translation_, move);
}

void Enemy::Fire() {
		// 弾があれば解放する(listを使うので必要なし)
		// 弾の速度
		const float kBulletSpeed = -1.0f;
		Vector3 velocity(0, 0, kBulletSpeed);

		// 速度ベクトルを敵機の向きに合わせて回転させる
		velocity = TransformNormal(velocity, worldTransform_.matWorld_);

		// 弾を生成し、初期化
		EnemyBullet* newBullet = new EnemyBullet();
		newBullet->Initialize(model_, worldTransform_.translation_, velocity);

		// 弾を登録する
		bullets_.push_back(newBullet);
}

void (Enemy::*Enemy::phaseTable[])() = {
	&Enemy::Approach,
	&Enemy::Leave
};