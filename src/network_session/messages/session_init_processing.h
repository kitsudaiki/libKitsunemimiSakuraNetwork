/**
 *  @file       session_init_processing.h
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#ifndef SESSION_INIT_PROCESSING_H
#define SESSION_INIT_PROCESSING_H

#include <network_session/session_handler.h>
#include <network_session/messages/message_definitions.h>
#include <network_session/messages/message_creation.h>
#include <abstract_socket.h>

#include <logger/logger.h>

using Kitsune::Network::AbstractSocket;

namespace Kitsune
{
namespace Project
{
namespace Common
{

/**
 * @brief processSessionInitStart
 */
inline void
processSessionInitStart(const Session_InitStart_Message* message,
                        AbstractSocket* socket)
{
    Persistence::LOG_debug("processSessionInitStart");

    const uint32_t sessionId = message->offeredSessionId;
    if(SessionHandler::m_sessionHandler->isIdUsed(sessionId))
    {
        const uint32_t newSessionId = SessionHandler::m_sessionHandler->increaseSessionIdCounter();

        Session newSession;
        newSession.socket = socket;
        newSession.sessionId = newSessionId;

        sendSessionIdChange(sessionId, newSessionId, socket);
        SessionHandler::m_sessionHandler->addPendingSession(newSessionId, newSession);
    }
    else
    {
        Session newSession;
        newSession.socket = socket;
        newSession.sessionId = sessionId;

        sendSessionInitReply(sessionId, socket);

        SessionHandler::m_sessionHandler->addSession(sessionId, newSession);
    }
}

/**
 * @brief processSessionIdChange
 */
inline void
processSessionIdChange(const Session_IdChange_Message* message,
                       AbstractSocket* socket)
{
    Persistence::LOG_debug("processSessionIdChange");

    const uint32_t sessionId = message->newOfferedSessionId;
    const uint32_t oldSessionId = message->oldOfferedSessionId;

    if(SessionHandler::m_sessionHandler->isIdUsed(sessionId))
    {
        const uint32_t newSessionId = SessionHandler::m_sessionHandler->increaseSessionIdCounter();

        sendSessionIdChange(sessionId, newSessionId, socket);

        Session session = SessionHandler::m_sessionHandler->removePendingSession(oldSessionId);
        SessionHandler::m_sessionHandler->addPendingSession(newSessionId, session);
    }
    else
    {
        Session session = SessionHandler::m_sessionHandler->removePendingSession(oldSessionId);
        SessionHandler::m_sessionHandler->addSession(sessionId, session);
    }
}

/**
 * @brief processSessionIdConfirm
 */
inline void
processSessionIdConfirm(const Session_IdConfirm_Message* message,
                        AbstractSocket* socket)
{
    Persistence::LOG_debug("processSessionIdConfirm");

    // TODO
}

/**
 * @brief processSessionInitReply
 */
inline void
processSessionInitReply(const Session_InitReply_Message* message,
                        AbstractSocket* socket)
{
    Persistence::LOG_debug("processSessionInitReply");

    const uint32_t sessionId = message->sessionId;

    Session session = SessionHandler::m_sessionHandler->removePendingSession(sessionId);
    SessionHandler::m_sessionHandler->addSession(sessionId, session);
}

} // namespace Common
} // namespace Project
} // namespace Kitsune

#endif // SESSION_INIT_PROCESSING_H
