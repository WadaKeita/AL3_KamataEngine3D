#include "RailCamera.h"
#include "Function.h"
#include "imgui.h"

void RailCamera::Initialize(const Vector3& position, const Vector3& rotate) {

	worldTransform_.Initialize();
	// ワールドトランスフォームの初期設定
	worldTransform_.translation_ = position;
	worldTransform_.rotation_ = rotate;
	// ビュープロジェクションの初期化
	viewProjection_.Initialize();
}

void RailCamera::Update() {

	Vector3 moveSpeed = {0, 0, 0};
	worldTransform_.translation_ = Add(worldTransform_.translation_, moveSpeed);
	worldTransform_.rotation_ = Add(worldTransform_.rotation_, rotateSpeed);

	worldTransform_.UpdateMatrix();

	// カメラオブジェクトのワールド行列からビュー行列を計算する
	viewProjection_.matView = Inverse(worldTransform_.matWorld_);

	// カメラの座標を画面表示する処理
	ImGui::Begin("Camera");

	ImGui::DragFloat3("position", &worldTransform_.translation_.x, 0.1f);
	ImGui::DragFloat3("rotation", &worldTransform_.rotation_.x, 0.1f);

	ImGui::DragFloat3("rotateSpeed", &rotateSpeed.x, 0.001f);

	ImGui::End();

}
