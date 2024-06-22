#include "GameScene.h"
#include "AxisIndicator.h"
#include "Enemy.h"
#include "Function.h"
#include "TextureManager.h"
#include "imgui.h"
#include <cassert>
#include <fstream>

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete model_;
	delete player_;
	// delete enemy_;
	delete debugCamera_;
	delete skydome_;
	delete modelSkydome_;

	for (Enemy* enemy : enemys_) {
		delete enemy;
	}
	for (EnemyBullet* bullet : enemyBullets_) {
		delete bullet;
	}
}

void GameScene::Initialize() {

	LoadEnemyPopData();

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	textureHandle_ = TextureManager::Load("uvChecker.png");
	model_ = Model::Create();
	worldTransform_.Initialize();
	viewProjection_.Initialize();

	// 自キャラの生成
	player_ = new Player();
	Vector3 playerPosition(0, 0, 40);
	// 自キャラの初期化
	player_->Initialize(model_, textureHandle_, playerPosition);

	textureHandle_ = TextureManager::Load("pa_Enemy.png");

	railCamera_ = new RailCamera();
	// レールカメラの生成
	railCamera_->Initialize({0, 0, 0}, {0, 0, 0});

	// 天球の生成
	skydome_ = new Skydome();
	modelSkydome_ = Model::CreateFromOBJ("AL_skydome", true);
	skydome_->Initialize(modelSkydome_);

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);
	// 軸方向表示を有効にする
	AxisIndicator::GetInstance()->SetVisible(true);
	// 軸方向表示が参照するビュープロジェクションを指定する（アドレス渡し）
	AxisIndicator::GetInstance()->SetTargetViewProjection(&viewProjection_);

	// 自キャラとレールカメラの親子関係を結ぶ
	player_->SetParent(&railCamera_->GetWorldTransform());

	// レティクルのテクスチャ
	TextureManager::Load("2Dreticle.png");

	waitTimer = 0;
}

void GameScene::Update() {

	UpdateEnemyPopCommands();

	// デスフラグの立った敵を削除
	enemys_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
	});

	// デスフラグの立った弾を削除
	enemyBullets_.remove_if([](EnemyBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	// レールカメラの更新
	railCamera_->Update();

	// 自キャラの更新
	player_->Update(viewProjection_);

	// 敵キャラの更新
	for (Enemy* enemy : enemys_) {
		enemy->Update();
	}

	// 弾更新
	for (EnemyBullet* bullet : enemyBullets_) {
		bullet->Update();
	}

	skydome_->Update();

	CheckAllCollisions();

#ifdef _DEBUG
	if (input_->TriggerKey(DIK_C)) {
		if (isDebugCameraActive_ == false) {
			isDebugCameraActive_ = true;
		} else {
			isDebugCameraActive_ = false;
		}
	}
	debugCamera_->Update();
#endif // _DEBUG

	// カメラの処理
	if (isDebugCameraActive_) {
		viewProjection_.matView = debugCamera_->GetViewProjection().matView;
		viewProjection_.matProjection = debugCamera_->GetViewProjection().matProjection;
		// ビュープロジェクション行列の転送
		viewProjection_.TransferMatrix();
	} else {
		viewProjection_.matView = railCamera_->GetViewProjection().matView;
		viewProjection_.matProjection = railCamera_->GetViewProjection().matProjection;
		// ビュープロジェクション行列の更新と転送
		viewProjection_.TransferMatrix();
	}

	ImGui::Begin("camera");

	ImGui::Checkbox("isDebugCamera", &isDebugCameraActive_);

	ImGui::End();
}

void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>

	// 自キャラの描画
	player_->Draw3D(viewProjection_);

	// 敵の描画
	for (Enemy* enemy : enemys_) {
		enemy->Draw(viewProjection_);
	}

	// 弾描画
	for (EnemyBullet* bullet : enemyBullets_) {
		bullet->Draw(viewProjection_);
	}

	skydome_->Draw(viewProjection_);

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	player_->DrawUI();

	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}

void GameScene::CheckAllCollisions() {
	// 判定対象AとBの座標
	Vector3 posA, posB;

	// 自弾リストの取得
	const std::list<PlayerBullet*>& playerBullets = player_->GetBullets();

#pragma region 自キャラと敵弾の当たり判定
	// 自キャラの座標
	posA = player_->GetWorldPosition();

	// 自キャラと敵弾全ての当たり判定
	for (EnemyBullet* bullet : enemyBullets_) {
		// 敵弾の座標
		posB = bullet->GetWorldPosition();

		// 2つの球の中心点間の距離を求める
		float distance = Length({posB.x - posA.x, posB.y - posA.y, posB.z - posA.z});

		// 球と球の交差判定
		if (distance <= player_->GetRadius() + bullet->GetRadius()) {
			// 自キャラの衝突時のコールバックを呼び出す
			player_->OnCollision();
			// 敵弾の衝突時のコールバックを呼び出す
			bullet->OnCollision();
		}
	}
#pragma endregion

#pragma region 自弾と敵キャラの当たり判定
	// 敵キャラの座標
	for (Enemy* enemy : enemys_) {

		posA = enemy->GetWorldPosition();

		// 自弾と敵キャラの当たり判定
		for (PlayerBullet* bullet : playerBullets) {
			// 自弾の座標
			posB = bullet->GetWorldPosition();

			// 2つの球の中心点間の距離を求める
			float distance = Length({posB.x - posA.x, posB.y - posA.y, posB.z - posA.z});

			// 球と球の交差判定
			if (distance <= enemy->GetRadius() + bullet->GetRadius()) {
				// 自キャラの衝突時のコールバックを呼び出す
				enemy->OnCollision();
				// 敵弾の衝突時のコールバックを呼び出す
				bullet->OnCollision();
			}
		}
	}
#pragma endregion

#pragma region 自弾と敵弾の当たり判定

	// 自弾と敵弾の当たり判定
	for (PlayerBullet* bulletA : playerBullets) {

		// 自弾の座標
		posA = bulletA->GetWorldPosition();

		for (EnemyBullet* bulletB : enemyBullets_) {

			// 敵弾の座標
			posB = bulletB->GetWorldPosition();

			// 2つの球の中心点間の距離を求める
			float distance = Length({posB.x - posA.x, posB.y - posA.y, posB.z - posA.z});

			// 球と球の交差判定
			if (distance <= bulletA->GetRadius() + bulletB->GetRadius()) {
				// 自キャラの衝突時のコールバックを呼び出す
				bulletA->OnCollision();
				// 敵弾の衝突時のコールバックを呼び出す
				bulletB->OnCollision();
			}
		}
	}

#pragma endregion
}

void GameScene::AddEnemyBullet(EnemyBullet* enemyBullet) {
	// リストに登録する
	enemyBullets_.push_back(enemyBullet);
}

void GameScene::AddEnemy(Vector3 pos) {

	// 敵の生成
	enemy_ = new Enemy();
	// 敵の初期化
	enemy_->Initialize(model_, textureHandle_,pos);
	// 敵キャラにゲームシーンを渡す
	enemy_->SetGameScene(this);
	// プレイヤーのアドレスをセットする
	enemy_->SetPlayer(player_);

	enemys_.push_back(enemy_);
}

void GameScene::LoadEnemyPopData() {
	
	// ファイルを開く
	std::ifstream file;
	file.open("Resources/enemyPop.csv");
	assert(file.is_open());

	// ファイルの内容を文字列ストリームにコピー
	enemyPopCommands << file.rdbuf();

	// ファイルを閉じる
	file.close();
}

void GameScene::UpdateEnemyPopCommands() {
	// 待機処理
	if (isWait == true) {
		waitTimer--;
		if (waitTimer <= 0) {
			// 待機完了
			isWait = false;
		}
		return;
	}
	// 1列分の文字列を入れる変数
	std::string line;

	// コマンド実行ループ
	while (getline(enemyPopCommands, line)) {
		// 1列分の文字列をストリームに変換して解析しやすくする
		std::istringstream line_stream(line);

		std::string word;
		// ,区切りで行の戦闘文字列を取得
		getline(line_stream, word, ',');

		// "//"からはじまる行はコメント
		if (word.find("//") == 0) {
			// コメント行を飛ばす
			continue;		
		}

		// POPコマンド
		if (word.find("POP") == 0) {
			// x座標
			getline(line_stream, word, ',');
			float x = (float)std::atof(word.c_str());

			// y座標
			getline(line_stream, word, ',');
			float y = (float)std::atof(word.c_str());

			// z座標
			getline(line_stream, word, ',');
			float z = (float)std::atof(word.c_str());

			// 敵を発生させる
			AddEnemy(Vector3(x, y, z));
		}

		// WAITコマンド
		else if (word.find("WAIT") == 0) {
			getline(line_stream, word, ',');

			// 待ち時間
			int32_t waitTime = atoi(word.c_str());

			// 待機開始
			isWait = true;
			waitTimer = waitTime;

			// コマンドループを抜ける
			break;
		}
	}
}
