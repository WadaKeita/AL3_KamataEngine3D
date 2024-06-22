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
	sprite2DReticle_ = Sprite::Create(textureReticle, {0, 0}, {0xff, 0xff, 0xff, 0xff}, {0.5f, 0.5f});
}

void Player::Update(const ViewProjection& viewProjection) {

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




	// 自機のワールド座標から3Dレティクルのワールド座標を計算
	// 自機から3Dレティクルへの距離
	const float kDistancePlayerTo3DReticle = 50.0f;
	// 時期から3Dレティクルへのオフセット（Z+向き）
	Vector3 offset = {0, 0, 1.0f};
	// 自機のワールド行列の回転を反映
	offset = TransformNormal(offset, worldTransform_.matWorld_);
	// ベクトルの長さを整える
	offset = Multiply(kDistancePlayerTo3DReticle, Normalize(offset));

	// 3Dレティクルの座標を設定
	worldTransform3DReticle_.translation_ = Add(worldTransform_.translation_, offset);
	worldTransform3DReticle_.UpdateMatrix();


	/// 3Dレティクルのワールド座標から2Dレティクルのスクリーン座標を計算

	Vector3 positionReticle = GetWorldPosition3DReticle();

	// ビューポート行列
	Matrix4x4 matViewport = MakeViewportMatrix(0, 0, WinApp::kWindowWidth, WinApp::kWindowHeight, 0, 1);

	// ビュー行列とプロジェクション行列、ビューポート行列を合成する
	Matrix4x4 matViewProjectionViewport = 
		Multiply(Multiply(viewProjection.matView, viewProjection.matProjection), matViewport);

	//positionReticle = TransformNormal(positionReticle, viewProjection_.matView);
	//positionReticle = TransformNormal(positionReticle, viewProjection_.matProjection);
	//positionReticle = TransformNormal(positionReticle, matViewport);

	// ワールド→スクリーン座標変換（ここで3Dから2Dになる）
	positionReticle = Transform(positionReticle, matViewProjectionViewport);

	// スプライトのレティクルに座標設定
	sprite2DReticle_->SetPosition(Vector2(positionReticle.x, positionReticle.y));


	ImGui::Begin("2dreticle");
	ImGui::DragFloat3("position", &positionReticle.x, 0.01f);

	ImGui::End();

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
	if (input_->TriggerKey(DIK_SPACE)) {
		// 弾があれば解放する(listを使うので必要なし)
		/*if (bullet_) {
		    delete bullet_;
		    bullet_ = nullptr;
		}*/
		Vector3 velocity;
		// 弾の速度
		const float kBulletSpeed = 1.0f;

		// 自機から照準オブジェクトへのベクトル
		velocity = Subtract(GetWorldPosition3DReticle(), GetWorldPosition());
		velocity = Multiply(kBulletSpeed, Normalize(velocity));

		// 速度ベクトルを自機の向きに合わせて回転させる
		velocity = TransformNormal(velocity, worldTransform_.matWorld_);

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
