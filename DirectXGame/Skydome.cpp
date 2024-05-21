#include "Skydome.h"
#include <assert.h>

void Skydome::Initialize(Model* model) {
	// NULLポインタチェック
	assert(model);

	model_ = model;

	worldTransform_.Initialize(); 
}

void Skydome::Update() { 
	worldTransform_.TransferMatrix(); 
}

void Skydome::Draw(const ViewProjection& viewProjection) {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, viewProjection);
}
