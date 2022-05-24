CREATE TABLE IF NOT EXISTS `account_bound_achievements` (
    `accountid` INT(10) UNSIGNED NOT NULL,
    `achievement` SMALLINT(5) UNSIGNED NOT NULL,
    `allowablerace` INT(11) NOT NULL,
    PRIMARY KEY (`accountid`, `achievement`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
