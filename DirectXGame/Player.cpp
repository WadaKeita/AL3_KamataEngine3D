#include "Player.h"
#include "Function.h"
#include "ImGuiManager.h"
#include "TextureManager.h"
#include "WinApp.h"
#include <cassert>

Player::~Player() {
	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	delete sprite2DReticle_;
}

void Player::Initialize(Model* model, uint32_t textureHandle, const Vector3& position) {
	// NULLポインタチェック
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;

	worldTransform_.Initialize();

	worldTransform_.translation_ = position;

	// シングルトンインスタンスを取得する
	input_ = Input::GetInstance();

	// 3Dレティクルのワールドトランスフォーム初期化
	worldTransform3DReticle_.Initialize();

	// 2Dレティクル用テクスチャ取得
	uint32_t textureReticle = TextureManager::Load("2Dreticle.png");

	// スプライト生成
	sprite2DReticle_ = Sprite::Create(textureReticle, {WinApp::kWindowWidth / 2, WinApp::kWindowHeight / 2}, {0xff, 0xff, 0xff, 0xff}, {0.5f, 0.5f});
}

void Player::Update(const ViewProjection& viewProjection) {

	viewProjection;

	Rotate();

	// デスフラグの立った弾を削除
	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	// キャラクターの移動ベクトル
	Vector3 move = {0, 0, 0};

	// キャラクターの移動速さ
	const float kCharacterSpeed = 0.2f;

	// ゲームパッドの状態を得る変数(XINPUT)
	XINPUT_STATE joyState;

	// ゲームパッド状態取得
	if (Input::GetInstance()->GetJoystickState(0, joyState)) {
		move.x += (float)joyState.Gamepad.sThumbLX / SHRT_MAX * kCharacterSpeed;
		move.y += (float)joyState.Gamepad.sThumbLY / SHRT_MAX * kCharacterSpeed;
	}

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

	// 移動限界座標
	const float kMoveLimitX = 34.5f;
	const float kMoveLimitY = 19.0f;

	// 範囲を越えない処理
	worldTransform_.translation_.x = max(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = min(worldTransform_.translation_.x, +kMoveLimitX);
	worldTransform_.translation_.y = max(worldTransform_.translation_.y, -kMoveLimitY);
	worldTransform_.translation_.y = min(worldTransform_.translation_.y, +kMoveLimitY);

	// 行列を定数バッァに転送
	worldTransform_.UpdateMatrix();

	//// 自機のワールド座標から3Dレティクルのワールド座標を計算
	//// 自機から3Dレティクルへの距離
	// const float kDistancePlayerTo3DReticle = 50.0f;
	//// 時期から3Dレティクルへのオフセット（Z+向き）
	// Vector3 offset = {0, 0, 1.0f};
	//// 自機のワールド行列の回転を反映
	// offset = TransformNormal(offset, worldTransform_.matWorld_);
	//// ベクトルの長さを整える
	// offset = Multiply(kDistancePlayerTo3DReticle, Normalize(offset));

	//// 3Dレティクルの座標を設定
	// worldTransform3DReticle_.translation_ = Add(worldTransform_.translation_, offset);
	// worldTransform3DReticle_.UpdateMatrix();

	// ScreenConversion(viewProjection);
	WorldConversion(viewProjection);

	Attack();

	// 弾更新
	for (PlayerBullet* bullet : bullets_) {
		bullet->Update();
	}

	// キャラクターの座標を画面表示する処理
	ImGui::Begin("Player");

	ImGui::DragFloat3("position", &worldTransform_.translation_.x, 0.1f);

	ImGui::End();
}

void Player::Draw3D(const ViewProjection& viewProjection) {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, viewProjection, textureHandle_);

	// 3Dレティクルを描画
	model_->Draw(worldTransform3DReticle_, viewProjection);

	// 弾描画
	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw(viewProjection);
	}
}

void Player::DrawUI() { sprite2DReticle_->Draw(); }

void Player::ScreenConversion(const ViewProjection& viewProjection) {
	/// 3Dレティクルのワールド座標から2Dレティクルのスクリーン座標を計算
	Vector3 positionReticle = GetWorldPosition3DReticle();

	// ビューポート行列
	Matrix4x4 matViewport = MakeViewportMatrix(0, 0, WinApp::kWindowWidth, WinApp::kWindowHeight, 0, 1);

	// ビュー行列とプロジェクション行列、ビューポート行列を合成する
	Matrix4x4 matViewProjectionViewport = Multiply(Multiply(viewProjection.matView, viewProjection.matProjection), matViewport);

	// positionReticle = TransformNormal(positionReticle, viewProjection_.matView);
	// positionReticle = TransformNormal(positionReticle, viewProjection_.matProjection);
	// positionReticle = TransformNormal(positionReticle, matViewport);

	// ワールド→スクリーン座標変換（ここで3Dから2Dになる）
	positionReticle = Transform(positionReticle, matViewProjectionViewport);

	// スプライトのレティクルに座標設定
	sprite2DReticle_->SetPosition(Vector2(positionReticle.x, positionReticle.y));

	ImGui::Begin("2dreticle");
	ImGui::DragFloat3("position", &positionReticle.x, 0.01f);

	ImGui::End();
}

void Player::WorldConversion(const ViewProjection& viewProjection) {

	// POINT mousePosition;

	// スプライトの現在座標を取得
	Vector2 spritePosition = sprite2DReticle_->GetPosition();

	XINPUT_STATE joyState;

	// ジョイスティック状態取得
	if (Input::GetInstance()->GetJoystickState(0, joyState)) {
		spritePosition.x += (float)joyState.Gamepad.sThumbRX / SHRT_MAX * 5.0f;
		spritePosition.y -= (float)joyState.Gamepad.sThumbRY / SHRT_MAX * 5.0f;

		spritePosition.x = min(max(spritePosition.x, 0), WinApp::kWindowWidth);
		spritePosition.y = min(max(spritePosition.y, 0), WinApp::kWindowHeight);

		// スプライトの座標変更を反映
		sprite2DReticle_->SetPosition(spritePosition);
	}

	//// マウス座標(スクリーン座標)を取得する
	// GetCursorPos(&mousePosition);

	//// クライアントエリア座標に変換する
	// HWND hwnd = WinApp::GetInstance()->GetHwnd();
	// ScreenToClient(hwnd, &mousePosition);

	// spritePosition = Vector2((float)mousePosition.x, (float)mousePosition.y);
	sprite2DReticle_->SetPosition(spritePosition);

	// ビューポート行列
	Matrix4x4 matViewport = MakeViewportMatrix(0, 0, WinApp::kWindowWidth, WinApp::kWindowHeight, 0, 1);

	// ビュープロジェクションビューポート合成行列
	Matrix4x4 matVPV = Multiply(Multiply(viewProjection.matView, viewProjection.matProjection), matViewport);
	// 合成行列の逆行列を計算する
	Matrix4x4 matInverseVPV = Inverse(matVPV);

	// スクリーン座標
	Vector3 posNear = Vector3((float)spritePosition.x, (float)spritePosition.y, 0);
	Vector3 posFar = Vector3((float)spritePosition.x, (float)spritePosition.y, 1);

	// スクリーン座標系からワールド座標系へ
	posNear = Transform(posNear, matInverseVPV);
	posFar = Transform(posFar, matInverseVPV);

	// マウスレイの方向
	Vector3 mouseDirection = Subtract(posFar, posNear);
	mouseDirection = Normalize(mouseDirection);
	// カメラから照準オブジェクトの距離
	const float kDistanceTestObject = 80.0f;
	worldTransform3DReticle_.translation_ = Add(posNear, Multiply(kDistanceTestObject, mouseDirection));

	worldTransform3DReticle_.UpdateMatrix();

	ImGui::Begin("Player");
	ImGui::Text("2DReticle:(%f,%f)", spritePosition.x, spritePosition.y);
	ImGui::Text("Near:(%+.2f,%+.2f,%+.2f)", posNear.x, posNear.y, posNear.z);
	ImGui::Text("Far:(%+.2f,%+.2f,%+.2f)", posFar.x, posFar.y, posFar.z);
	ImGui::Text("3DReticle:(%+.2f,%+.2f,%+.2f)", worldTransform3DReticle_.translation_.x, worldTransform3DReticle_.translation_.y, worldTransform3DReticle_.translation_.z);

	ImGui::End();
}

void Player::Rotate() {
	// 回転速さ[ラジアン/frame]
	const float kRotSpeed = 0.02f;

	// 押した方向で移動ベクトルを変更
	if (input_->PushKey(DIK_A)) {
		worldTransform_.rotation_.y -= kRotSpeed;
	} else if (input_->PushKey(DIK_D)) {
		worldTransform_.rotation_.y += kRotSpeed;
	}
}

void Player::Attack() {
	//if (input_->TriggerKey(DIK_SPACE)) {
	//	// 弾があれば解放する(listを使うので必要なし)
	//	/*if (bullet_) {
	//	    delete bullet_;
	//	    bullet_ = nullptr;
	//	}*/
	//	Vector3 velocity;
	//	// 弾の速度
	//	const float kBulletSpeed = 1.0f;

	//	// 自機から照準オブジェクトへのベクトル
	//	velocity = Subtract(GetWorldPosition3DReticle(), GetWorldPosition());
	//	velocity = Multiply(kBulletSpeed, Normalize(velocity));


	//	// 弾を生成し、初期化
	//	PlayerBullet* newBullet = new PlayerBullet();
	//	newBullet->Initialize(model_, GetWorldPosition(), velocity);

	//	// 弾を登録する
	//	bullets_.push_back(newBullet);
	//}


	XINPUT_STATE joyState;
	// ゲームパッド未接続なら何もせず抜ける
	if (!Input::GetInstance()->GetJoystickState(0, joyState)) {
		return;
	}

	// Rトリガーを押していたら
	if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {

		Vector3 velocity;
		// 弾の速度
		const float kBulletSpeed = 1.0f;

		// 自機から照準オブジェクトへのベクトル
		velocity = Subtract(GetWorldPosition3DReticle(), GetWorldPosition());
		velocity = Multiply(kBulletSpeed, Normalize(velocity));

		// 弾を生成し、初期化
		PlayerBullet* newBullet = new PlayerBullet();
		newBullet->Initialize(model_, GetWorldPosition(), velocity);

		// 弾を登録する
		bullets_.push_back(newBullet);
	}

}

Vector3 Player::GetWorldPosition() {

	// ワールド座標を入れる変数
	Vector3 worldPos;

	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

Vector3 Player::GetWorldPosition3DReticle() {

	// ワールド座標を入れる変数
	Vector3 worldPos;

	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform3DReticle_.matWorld_.m[3][0];
	worldPos.y = worldTransform3DReticle_.matWorld_.m[3][1];
	worldPos.z = worldTransform3DReticle_.matWorld_.m[3][2];

	return worldPos;
}

void Player::OnCollision() {}

void Player::SetParent(const WorldTransform* parent) {
	// 親子関係を結ぶ
	worldTransform_.parent_ = parent;
}
