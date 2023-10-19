#ifndef __SF_CLOUD_HPP__
#define __SF_CLOUD_HPP__
namespace sf
{
    namespace cloud
    {
        typedef enum error_
        {
            SUCCESS,
            ATTEMPTS_EXCEEDED,
            TIMEOUT,
        }error_e;

        /**
         * @brief Connects and waits for the system to complete the connection to cloud
         * services.
         * 
         * @param timeout_ms Timeout in milliseconds.
         * @return 0 on success, otherwise error code.
         */
        int wait_connect(int timeout_ms);
        /**
         * @brief Disconnects and waits for the system to complete the disconnect from
         * cloud services.
         * 
         * @param timeout_ms Timeout in milliseconds
         * @return 0 on success, otherwise error code.
         */
        int wait_disconnect(int timeout_ms);

        /**
         * @brief Initializes the counter.
         * 
         * If the counter is greater than SF_CLOUD_CONNECT_MAX_ATTEMPTS, then 
         * FLOG_UPL_CONNECT_FAIL is recorded.
         * 
         * Returns true if no error detection, otherwise false if the firmware
         * should abort the next upload
         * 
         * @return false if in an error condition, otherwise true
         */
        bool initialize_counter(void);
    };
};
#endif