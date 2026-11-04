// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "cellapp.h"
#include "entity.h"
#include "move_controller.h"
#include "moveto_point_handler.h"
#include "moveto_entity_handler.h"
#include "navigate_handler.h"

namespace KBEngine{


//-------------------------------------------------------------------------------------
MoveController::MoveController(Entity* pEntity, MoveToPointHandler* pMoveToPointHandler, uint32 id):
Controller(Controller::CONTROLLER_TYPE_MOVE, pEntity, 0, id),
pMoveToPointHandler_(pMoveToPointHandler)
{
}

//-------------------------------------------------------------------------------------
MoveController::~MoveController()
{
	// DEBUG_MSG(fmt::format("MoveController::~MoveController(): {:p}\n", (void*)this);
	if (pMoveToPointHandler_)
	{
		pMoveToPointHandler_->destroy();
		pMoveToPointHandler_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
void MoveController::addToStream(KBEngine::MemoryStream& s)
{
	Controller::addToStream(s);

	if (pMoveToPointHandler_ == NULL)
	{
		// 非法 move type 已由 createFromStream 拒绝（该实体不会被序列化回传），
		// 此处防御性返回，避免解引用空指针。
		ERROR_MSG("MoveController::addToStream: no move handler, skip serialization!\n");
		return;
	}

	uint8 utype = pMoveToPointHandler_->type();
	s << utype;

	pMoveToPointHandler_->addToStream(s);
}

//-------------------------------------------------------------------------------------
void MoveController::createFromStream(KBEngine::MemoryStream& s)
{
	Controller::createFromStream(s);
	KBE_ASSERT(pMoveToPointHandler_ == NULL);

	uint8 utype;
	s >> utype;

	if(utype == MoveToPointHandler::MOVE_TYPE_NAV)
		pMoveToPointHandler_ = new NavigateHandler();
	else if(utype == MoveToPointHandler::MOVE_TYPE_ENTITY)
		pMoveToPointHandler_ = new MoveToEntityHandler();
	else if(utype == MoveToPointHandler::MOVE_TYPE_POINT)
		pMoveToPointHandler_ = new MoveToPointHandler();
	else
	{
		ERROR_MSG(fmt::format("MoveController::createFromStream: invalid move type({})!\n",
			(int)utype));
		// 非法类型：不初始化 handler 并置标记，Entity::createMovementHandlerFromStream
		// 将中止该实体的反序列化，避免后续流读取错位。
		invalidType_ = true;
		return;
	}

	pMoveToPointHandler_->createFromStream(s);
}

//-------------------------------------------------------------------------------------
void MoveController::destroy()
{
	Controller::destroy();

	// 既然自己要销毁了，那么与自己相联的updatable也应该停止了
	if (pMoveToPointHandler_)
	{
		pMoveToPointHandler_->destroy();
		pMoveToPointHandler_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
}

