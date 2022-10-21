CREATE TABLE IF NOT EXISTS `account_bound_linked_accounts` (
    `account_id` INT(10) UNSIGNED NOT NULL,
    `linked_account_id` INT(10) UNSIGNED NOT NULL,
    PRIMARY KEY (`account_id`,`linked_account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
