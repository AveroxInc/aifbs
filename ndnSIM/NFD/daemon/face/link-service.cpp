/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2017,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "link-service.hpp"
#include "face.hpp"

namespace nfd {
namespace face {

NFD_LOG_INIT("LinkService");

LinkService::LinkService()
  : m_face(nullptr)
  , m_transport(nullptr)
{
}

LinkService::~LinkService()
{
}

void
LinkService::setFaceAndTransport(Face& face, Transport& transport)
{
  BOOST_ASSERT(m_face == nullptr);
  BOOST_ASSERT(m_transport == nullptr);

  m_face = &face;
  m_transport = &transport;
}

void LinkService::sendInterest(const Interest& interest) {
	BOOST_ASSERT(m_transport != nullptr);
	NFD_LOG_FACE_TRACE(__func__);

	LogManager::AddLogWithNodeId("link-service.cpp->sendInterest.start");
	LogManager::AddLogWithNodeId("link-service.cpp->sendInterest.accNames", NameHelper::join(interest.getAccNames()));

	BFController bfController;
	bfController.startInterestTimer();
	bfController.checkInterest(NameHelper::join(interest.getAccNames()));

	if (bfController.shouldFloodInterest()) {

		++this->nOutInterests;

		doSendInterest(interest);

		afterSendInterest(interest);

	}

	LogManager::AddLogWithNodeId("link-service.cpp->sendInterest.completed");

	//Working code (before Blind Forwarding)
	/*
	 BOOST_ASSERT(m_transport != nullptr);
	 NFD_LOG_FACE_TRACE(__func__);

	 LogManager::AddLogWithNodeId("link-service.cpp->sendInterest.start");
	 //  LogManager::AddLogWithNodeId("link-service.cpp->sendInterest.name", interest.getName().toUri());
	 LogManager::AddLogWithNodeId("link-service.cpp->sendInterest.accNames", NameHelper::join(interest.getAccNames()));

	 ++this->nOutInterests;

	 doSendInterest(interest);

	 afterSendInterest(interest);

	 LogManager::AddLogWithNodeId("link-service.cpp->sendInterest.completed");
	 */
}

void LinkService::sendData(const Data& data) {
	BOOST_ASSERT(m_transport != nullptr);
	NFD_LOG_FACE_TRACE(__func__);

	LogManager::AddLogWithNodeId("link-service.cpp->sendData.start");
	LogManager::AddLogWithNodeId("link-service.cpp->sendData.name", data.getName().toUri());

	BFController bfController;
	bfController.startDataTimer();
	bfController.checkData(data.getName().toUri());

	if (bfController.shouldFloodData()) {

		++this->nOutData;

		doSendData(data);

		afterSendData(data);

	}

	LogManager::AddLogWithNodeId("link-service.cpp->sendData.completed");

	//Working code (before Blind-Forwarding
	/*
	 BOOST_ASSERT(m_transport != nullptr);
	 NFD_LOG_FACE_TRACE(__func__);


	 LogManager::AddLogWithNodeId("link-service.cpp->sendData.start");
	 LogManager::AddLogWithNodeId("link-service.cpp->sendData.name", data.getName().toUri());

	 ++this->nOutData;

	 doSendData(data);

	 afterSendData(data);

	 LogManager::AddLogWithNodeId("link-service.cpp->sendData.completed");
	 */
}

void
LinkService::sendNack(const ndn::lp::Nack& nack)
{
  BOOST_ASSERT(m_transport != nullptr);
  NFD_LOG_FACE_TRACE(__func__);

  ++this->nOutNacks;

  doSendNack(nack);

  afterSendNack(nack);
}

void
LinkService::receiveInterest(const Interest& interest) {

	NFD_LOG_FACE_TRACE(__func__);

	LogManager::AddLogWithNodeId("link-service.cpp->receiveInterest.start");
	LogManager::AddLogWithNodeId("link-service.cpp->receiveInterest.previousNodeId", interest.getPreviousNodeId());
//	LogManager::AddLogWithNodeId("link-service.cpp->receiveInterest.name", interest.getName().toUri());
	LogManager::AddLogWithNodeId("link-service.cpp->receiveInterest.accNames", NameHelper::join(interest.getAccNames()));

	unsigned int size = interest.getAccNames().size();
	LogManager::AddLogWithNodeId("link-service.cpp->receiveInterest.size", size);

	if(size == 0){
		++this->nInInterests;
		afterReceiveInterest(interest);
	}
	else{
		for (uint32_t i = 0; i < size; i++) {
			std::vector<Name> v = interest.getAccNames();
			Name n = v[i];
			auto tempInterest = std::make_shared<Interest>(interest.wireEncode());
			const_cast<Interest&>(*tempInterest).setName(n);
			LogManager::AddLogWithNodeId("link-service.cpp->receiveInterest.loop.name", const_cast<Interest&>(*tempInterest).getName().toUri());
			++this->nInInterests;
			afterReceiveInterest(*tempInterest);
		}
	}

	LogManager::AddLogWithNodeId("link-service.cpp->receiveInterest.completed");

	//Working code (before accumulation)
/*
	 NFD_LOG_FACE_TRACE(__func__);

	 LogManager::AddLogWithNodeId("link-service.cpp->receiveInterest.start");
	 LogManager::AddLogWithNodeId("link-service.cpp->receiveInterest.name", interest.getName().toUri());
	 LogManager::AddLogWithNodeId("link-service.cpp->receiveInterest.previousNodeId", interest.getPreviousNodeId());
	 LogManager::AddLogWithNodeId("link-service.cpp->receiveInterest.accNames", NameHelper::join(interest.getAccNames()));

	 ++this->nInInterests;

	 afterReceiveInterest(interest);

	 LogManager::AddLogWithNodeId("link-service.cpp->receiveInterest.completed");
*/
}

void
LinkService::receiveData(const Data& data)
{
  NFD_LOG_FACE_TRACE(__func__);

  LogManager::AddLogWithNodeId("link-service.cpp->receiveData.start");
  LogManager::AddLogWithNodeId("link-service.cpp->receiveData.name", data.getName().toUri());

  ++this->nInData;

  afterReceiveData(data);

  LogManager::AddLogWithNodeId("link-service.cpp->receiveData.completed");
}

void
LinkService::receiveNack(const ndn::lp::Nack& nack)
{
  NFD_LOG_FACE_TRACE(__func__);

  LogManager::AddLogWithNodeId("link-service.cpp->receiveNack.start");

  ++this->nInNacks;

  afterReceiveNack(nack);

  LogManager::AddLogWithNodeId("link-service.cpp->receiveNack.completed");
}

void
LinkService::notifyDroppedInterest(const Interest& interest)
{
  ++this->nDroppedInterests;
  onDroppedInterest(interest);
}

std::ostream&
operator<<(std::ostream& os, const FaceLogHelper<LinkService>& flh)
{
  const Face* face = flh.obj.getFace();
  if (face == nullptr) {
    os << "[id=0,local=unknown,remote=unknown] ";
  }
  else {
    os << "[id=" << face->getId() << ",local=" << face->getLocalUri()
       << ",remote=" << face->getRemoteUri() << "] ";
  }
  return os;
}

} // namespace face
} // namespace nfd
