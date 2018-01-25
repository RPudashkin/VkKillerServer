#ifndef VKKILLER_CLIENT_H
#define VKKILLER_CLIENT_H

#include <QTcpSocket>

class VkKillerServer;
class VkKillerTopic;


class VkKillerClient: private QTcpSocket {
    friend class VkKillerServer;

public:
    VkKillerClient(qintptr socketDescriptor, QObject* parent = nullptr);
   ~VkKillerClient();

    VkKillerClient(const VkKillerClient&) 			 = delete;
    VkKillerClient(VkKillerClient&&) 	  			 = delete;
    VkKillerClient& operator=(const VkKillerClient&) = delete;
    VkKillerClient& operator=(VkKillerClient&&) 	 = delete;

    QString name() const noexcept;

    // 32 characters per name this is maximum
    static constexpr quint8 MAX_NAME_LENGTH = 32;

private:
    QString 		m_name;
    VkKillerTopic*  m_selectedTopic;
};

#endif // VKKILLER_CLIENT_H