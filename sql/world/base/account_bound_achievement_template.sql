DROP TABLE IF EXISTS `account_bound_achievement_template`;
CREATE TABLE `account_bound_achievement_template` (
    `id` INT(10) UNSIGNED NOT NULL,
    `allowablerace` INT(11) NOT NULL,
    `comment` VARCHAR(255) NOT NULL,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
