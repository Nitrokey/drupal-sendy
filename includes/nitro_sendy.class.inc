<?php
class nitro_sendy

{
    static private $TABLE = "nitro_sendy";
    static private $SUBSCRIBE = "subscribe";
    static private $UNSUBSCRIBE = "unsubscribe";
    static private $STATUS = "api/subscribers/subscription-status.php";
    static private $COUNT = "api/subscribers/active-subscriber-count.php";
    static private $CREATE = "api/campaigns/create.php";


    /**
     * Makes a POST request to deliver the created newsletter to Sendy
     *
     * @param string $title
     *   Contains the email subject to be used in the final newsletter
     * @param string $html
     *   Contains the whole newsletter as html
     */
    static public function nitro_sendy_post($title, $html)
    {
        // load settings

        $data = self::nitro_sendy_load_settings();
        if (self::nitro_sendy_module_active($data['active'])) {
            $url = self::nitro_sendy_create_url($data['url'], self::$CREATE);
            $subject = $title;
            if ($data['subject'] != "") $subject = $data['subject'] . ' ' . $subject;

            // set POST variables

            $fields = array(
                'api_key' => $data['api_key'],
                'from_name' => $data['from_name'],
                'from_email' => $data['from_email'],
                'reply_to' => $data['reply_to'],
                'subject' => $subject,
                'html_text' => $html,
                'list_ids' => $data['list_id'],
                'send_campaign' => $data['send_campaign']
            );
            self::nitro_sendy_process_curl($url, $fields);
        }
    }

    /**
     * Saves settings to the database
     *
     * @param array $data
     *   Contains the data array to be stored in the database
     */
    static public function nitro_sendy_save_settings($data)
    {
        $result = db_select(self::$TABLE, 't')->fields('t', array(
            'id'
        ))->execute();
        $num_of_results = $result->rowCount();
        if (!isset($data['active'])) $data['active'] = 0;
        if ($num_of_results === 0) {
            db_insert(self::$TABLE)->fields(array(
                'active' => $data['active'],
                'api_key' => $data['api'],
                'url' => $data['url'],
                'from_name' => $data['from_name'],
                'from_email' => $data['from_email'],
                'reply_to' => $data['reply_to'],
                'subject' => $data['subject'],
                'list_id' => $data['list_id'],
                'list_id_subscriber' => $data['list_id_subscriber'],
                'list_id_check' => $data['list_id_check'],
                'subscriber_number' => $data['subscriber_number'],
                'brand_id' => $data['brand_id'],
                'send_campaign' => $data['send_campaign'],
            ))->execute();
        }
        else {
            $id = $result->fetchAssoc() ['id'];
            db_update(self::$TABLE)->condition('id', $id, '=')->fields(array(
                'active' => $data['active'],
                'api_key' => $data['api'],
                'url' => $data['url'],
                'from_name' => $data['from_name'],
                'from_email' => $data['from_email'],
                'reply_to' => $data['reply_to'],
                'subject' => $data['subject'],
                'list_id' => $data['list_id'],
                'list_id_subscriber' => $data['list_id_subscriber'],
                'list_id_check' => $data['list_id_check'],
                'subscriber_number' => $data['subscriber_number'],
                'brand_id' => $data['brand_id'],
                'send_campaign' => $data['send_campaign'],
            ))->execute();
        }
    }

    /**
     * Loads settings from the database
     *
     * @return array
     *   Contains the settings
     */
    static public function nitro_sendy_load_settings()
    {
        return db_select(self::$TABLE, 'e')->fields('e', array(
            'active',
            'api_key',
            'url',
            'from_name',
            'from_email',
            'reply_to',
            'subject',
            'list_id',
            'list_id_subscriber',
            'list_id_check',
            'subscriber_number',
            'brand_id',
            'send_campaign'
        ))->execute()->fetchAssoc();
    }

    /**
     * Makes a POST request to subscribe a user to the predefined list
     *
     * @param string $email
     *   Contains the email of the subscriber
     * @param string $name
     *   Contains the name of the subscriber
     * @param bool $response
     *   Decides wether to get a plain text response or not
     * @return string
     *   Contains the result message
     */
    static public function nitro_sendy_subscribe($email, $name = "", $response = false, $referrer = "")
    {
        $data = self::nitro_sendy_load_settings();
        $status = self::nitro_sendy_module_active($data['active']);

        // if not active

        if ($status !== true) return $status;
        else {
            $url = self::nitro_sendy_create_url($data['url'], self::$SUBSCRIBE);
            if ($response) $boolean = "true";
            else $boolean = "false";
            $fields = array(
                'name' => $name,
                'email' => $email,
                'referrer' => $referrer,
                'list' => $data['list_id_subscriber'],
                'boolean' => $boolean,
            );
            $result = self::nitro_sendy_process_curl($url, $fields);
            if ($response) {
                switch ($result) {
                    case '1':
                        return t("Thank you for signing up to our newsletter. You will receive a confirmation email shortly.");
                    default:
                        return $result;
                }
            }
        }
    }

    /**
     * Makes a POST request to unsubscribe a user from the predefined list
     *
     * @param string $email
     *   Contains the email of the subscriber
     * @param string $list
     *   Contains the list id of which to unsubsribe the user
     * @param bool $response
     *   Decides wether to get a plain text response or not
     * @return string
     *   Contains the result message
     */
    static public function nitro_sendy_unsubscribe($email, $list, $response = false)
    {
        $data = self::nitro_sendy_load_settings();
        $status = self::nitro_sendy_module_active($data['active']);

        // if not active

        if ($status !== true) return $status;
        else {
            $url = self::nitro_sendy_create_url($data['url'], self::$UNSUBSCRIBE);
            if ($response) $boolean = "true";
            else $boolean = "false";
            $fields = array(
                'email' => $email,
                'list' => $list,
                'boolean' => $boolean,
            );
            $result = self::nitro_sendy_process_curl($url, $fields);
            if ($response) return $result;
        }
    }

    /**
     * Makes a POST request to unsubscribe a user from the predefined list
     *
     * @param string $email
     *   Contains the email of the subscriber
     * @param string $list
     *   Contains the list id of which to unsubsribe the user
     * @return string
     *   Contains the result message
     */
    static public function nitro_sendy_subscriber_status($email, $list)
    {
        $data = self::nitro_sendy_load_settings();
        $status = self::nitro_sendy_module_active($data['active']);

        // if not active

        if ($status != true) return $status;
        else {
            $url = self::nitro_sendy_create_url($data['url'], self::$STATUS);
            $fields = array(
                'api_key' => $data['api_key'],
                'email' => $email,
                'list' => $list,
            );
            return self::nitro_sendy_process_curl($url, $fields);
        }
    }

    /**
     * Makes a POST request to retrieve a number of subscribed users for the provided list id
     *
     * @param string $list
     *   Contains the list id for which to get the number of subscribers
     * @return string
     *   Contains the result message
     */
    static public function nitro_sendy_active_subscriber_count($list)
    {
        $data = self::nitro_sendy_load_settings();
        $status = self::nitro_sendy_module_active($data['active']);

        // if not active

        if ($status != true) return $status;
        else {
            $url = self::nitro_sendy_create_url($data['url'], self::$COUNT);
            $fields = array(
                'api_key' => $data['api_key'],
                'list_id' => $list,
            );
            $result = self::nitro_sendy_process_curl($url, $fields);
            $number = $result;
            if (!is_numeric($number)) $number = 0;
            self::nitro_sendy_set_subscribers($list, $number);
            return $result;
        }
    }

    /**
     *  Creates url out of a base and a file.
     *
     * @param string $url
     *   Contains the base url
     * @param string $file
     *   Contains the file
     * @return string
     *   Contains the new url
     */
    static private function nitro_sendy_create_url($url, $file)
    {
        if (substr($url, -1) != '/') $url = $url . '/';
        $url = $url . $file;
        return $url;
    }

    /**
     * CURL function to post the given fields to the given url
     *
     * @param string $url
     *   Contains the post destination
     * @param array $data
     *   Contains the fields which are being posted
     * @return string
     *   Contains the result message
     */
    static private function nitro_sendy_process_curl($url, $data)
    {

        // open connection

        $ch = curl_init();

        // set the url, number of POST vars, POST data

        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_POST, count($data));
        curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
        ob_start();

        // execute post

        curl_exec($ch);
        $result = ob_get_contents();
        ob_end_clean();

        // close connection

        curl_close($ch);
        return $result;
    }

    /**
     * Creates and returns a submit form
     *
     * @param string $language
     *   Defines the language being used for the form.
     * @return string
     *   Contains the form.
     */
    static public function nitro_sendy_create_subscribe_form($language = "en")
    {
        $email = t("Your E-Mail Address");
        $button = t("Subscribe");
        if ($language == "de") {
            $email = "Ihre E-Mail Adresse";
            $button = "Anmelden";
        }

        if (isset($_SESSION['ref']))
            $referrer = $_SESSION['ref'];
        else
            $referrer = "";


        $form = '<form id="newsletter-subscription-form" enctype="multipart/form-data" action="/" method="post" accept-charset="UTF-8">' .
                    '<div class="newsletter-subscription">' .
                        '<input type="email" id="submitted-email" name="email" size="60" placeholder="' . $email . '">' .
                        '<input type="submit" name="nitro_submit" value="' . $button . '">' .
                        '<input type="hidden" name="ref" value="' . $referrer . '">' .
                        '</div>' .
                        '</form>';
        return '<p class="newsletter-status">' . self::nitro_sendy_submit() . '</p>' . $form;
    }

    static public function nitro_sendy_submit()
    {
        if (isset($_POST['nitro_submit']) && $_POST['nitro_submit'] == "Subscribe")
            if (isset($_POST['nitro_submit']) && valid_email_address($_POST['email'])) return self::nitro_sendy_subscribe($_POST['email'], "", true, $_POST['ref']);
    }

    /**
     * Creates and returns a submit form
     *
     * @param string $status
     *   Contains the module state. ("0" or "1")
     * @return string
     *   returns true if the module is active, otherwise returns a message to work with.
     */
    static private function nitro_sendy_module_active($status)
    {
        if ($status === '1') return true;
        else return t("This function is currently disabled. Please try again in a few minutes.");
    }

    /**
     * Saves the checked list id and the number of subscribers for that list.
     *
     * @param string $list
     *   Contains the list id.
     * @param string $number
     *   Contains the number of subscribers.
     */
    static public function nitro_sendy_set_subscribers($list, $number)
    {
        db_update(self::$TABLE)->fields(array(
            'list_id_check' => $list,
            'subscriber_number' => $number,
        ))->execute();
    }

    /**
     * Get the number of subscribers from last time.
     *
     * @return string
     *   Contains the number of subscribers from the last check.
     */
    static public function nitro_sendy_get_subscribers()
    {
        return db_select(self::$TABLE, 'e')->fields('e', array(
            'subscriber_number'
        ))->execute()->fetchAssoc();
    }
}
