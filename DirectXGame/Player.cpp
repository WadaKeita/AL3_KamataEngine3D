#include "Player.h"
#include <cassert>
#include "Function.h"
#include "ImGuiManager.h"

void Player::Initialize(Model* model, uint32_t textureHandle) {
	// NULLポインタチェック
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;

	worldTransform_.Initialize();

	// シングルトンインスタンスを取得する
	input_ = Input::GetInstance();
}

void Player::Update() {
	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();

	// キャラクターの移動ベクトル
	Vector3 move = {0, 0, 0};

	// キャラクターの移動速さ
	const float kCharacterSpeed = 0.2f;

	// 押した方向で移動ベクトルを変更（左右）
	if (input_->PushKey(DIK_LEFT)) {
		move.x -= kCharacterSpeed;
	} else if (input_->PushKey(DIK_RIGHT)) {
		move.x += kCharacterSpeed;
	}
	// 押した方向で移動ベクトルを変更（上下）
	if (input_->PushKey(DIK_DOWN)) {
		move.y -= kCharacterSpeed;
	} else if (input_->PushKey(DIK_UP)) {
		move.y += kCharacterSpeed;
	}

	// 座標移動
	worldTransform_.translation_ = Add(worldTransform_.translation_, move);
	
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	
	// 移動限界座標
	const float kMoveLimitX = 30.0f;
	const float kMoveLimitY = 20.0f;

	// 範囲を越えない処理
	worldTransform_.translation_.x = max(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = min(worldTransform_.translation_.x, +kMoveLimitX);
	worldTransform_.translation_.y = max(worldTransform_.translation_.y, -kMoveLimitY);
	worldTransform_.translation_.y = min(worldTransform_.translation_.y, +kMoveLimitY);

	// キャラクターの座標を画面表示する処理
	ImGui::Begin("Player");

	ImGui::DragFloat3("position", &worldTransform_.translation_.x, 0.1f);

	ImGui::End();
}

void Player::Draw(ViewProjection& viewProjection) {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, viewProjection, textureHandle_);
}

