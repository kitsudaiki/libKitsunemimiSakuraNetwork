/**
 *  @file       session.cpp
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  Apache License Version 2.0
 */

#include "session.h"

namespace Kitsune
{
namespace Project
{
namespace Common
{

/**
 * @brief Session::Session
 */
Session::Session()
{

}

/**
 * @brief Session::~Session
 */
Session::~Session()
{
    // TODO: send finish-message
    // TODO: delete all from timer-thread
    // TODO: close socket
}

} // namespace Common
} // namespace Project
} // namespace Kitsune
