/*
            This file is part of: 
                NoahFrame
            https://github.com/ketoo/NoahGameFrame

   Copyright 2009 - 2019 NoahFrame(NoahGameFrame)

   File creator: lvsheng.huang
   
   NoahFrame is open-source software and you can redistribute it and/or modify
   it under the terms of the License; besides, anyone who use this file/software must include this copyright announcement.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "NFActor.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"

NFActor::NFActor(const NFGUID id, NFIActorModule* pModule)
{
	this->id = id;
	m_pActorModule = pModule;
}

NFActor::~NFActor()
{
}

const NFGUID NFActor::ID()
{
	return this->id;
}

bool NFActor::Execute()
{
	//bulk
	NFActorMessage messageObject;
	while (mMessageQueue.TryPop(messageObject))
	{
		//must make sure that only one thread running this function at the same time
		//mxProcessFunctor is not thread-safe
		NFLock lock;

		lock.lock();
		ACTOR_PROCESS_FUNCTOR_PTR xBeginFunctor = mxProcessFunctor.GetElement(messageObject.msgID);
		lock.unlock();

		if (xBeginFunctor != nullptr)
		{
			xBeginFunctor->operator()(messageObject);

			//return the result to the main thread
			m_pActorModule->AddResult(messageObject);
		}
	}

	return true;
}

bool NFActor::AddComponent(NF_SHARE_PTR<NFIComponent> pComponent)
{
	//if you want to add more components for the actor, please don't clear the component
	//mxComponent.ClearAll();
	if (!mxComponent.ExistElement(pComponent->GetComponentName()))
	{
		mxComponent.AddElement(pComponent->GetComponentName(), pComponent);
		pComponent->SetActor(NF_SHARE_PTR<NFIActor>(this));

		pComponent->Awake();
		pComponent->Init();
		pComponent->AfterInit();
		pComponent->ReadyExecute();

		return true;
	}

	return false;
}

bool NFActor::RemoveComponent(const std::string& strComponentName)
{
	return false;
}

NF_SHARE_PTR<NFIComponent> NFActor::FindComponent(const std::string & strComponentName)
{
	return mxComponent.GetElement(strComponentName);
}

bool NFActor::AddMessageHandler(const int nSubMsgID, ACTOR_PROCESS_FUNCTOR_PTR xBeginFunctor)
{
	return mxProcessFunctor.AddElement(nSubMsgID, xBeginFunctor);
}

bool NFActor::SendMsg(const NFActorMessage& message)
{
	return mMessageQueue.Push(message);
}