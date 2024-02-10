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
            NOT_CONNECTED,
            OVERSIZE_DATA,
            OVERSIZE_NAME,
            PUBLISH_FAIL,
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
         * @brief Checks if system is still connected
         * 
         * @return true if connected, otherwise false
         */
        bool is_connected();

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

        /**
         * @brief Publishes the specified message
         * 
         * This function may block for as much as 20 seconds.
         *
         * @param title Message title.  Must be 1-64 characters and only letters,
         *                              numbers, underscores, dashes, or slashes.
         * @param blob Message blob.  Must be 1024 bytes or less, UTF-8 encoded.
         * @return 0 on success, otherise error code.
         */
        int publish_blob(const char* title, const char* blob);
    };
};
#endif