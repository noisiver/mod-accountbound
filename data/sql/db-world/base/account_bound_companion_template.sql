DROP TABLE IF EXISTS `account_bound_companion_template`;
CREATE TABLE `account_bound_companion_template` (
    `spell_id` INT(10) UNSIGNED NOT NULL,
    `allowable_race` INT(11) NOT NULL,
    `comment` VARCHAR(255) NOT NULL,
    PRIMARY KEY (`spell_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO `account_bound_companion_template` (`spell_id`, `allowable_race`, `comment`) VALUES
(4055, 1791, 'Mechanical Squirrel'),
(10673, 1791, 'Bombay Cat'),
(10674, 1791, 'Cornish Rex Cat'),
(10675, 1791, 'Black Tabby Cat'),
(10676, 1791, 'Orange Tabby Cat'),
(10677, 1791, 'Siamese Cat'),
(10678, 1791, 'Silver Tabby Cat'),
(10679, 1791, 'White Kitten'),
(10680, 1791, 'Cockatiel'),
(10681, 1791, 'Summon Cockatoo'),
(10682, 1791, 'Hyacinth Macaw'),
(10683, 1791, 'Green Wing Macaw'),
(10684, 1791, 'Senegal'),
(10685, 1791, 'Ancona Chicken'),
(10686, 1791, 'Summon Prairie Chicken'),
(10687, 1791, 'Summon White Plymouth Rock'),
(10688, 1791, 'Cockroach'),
(10695, 1791, 'Dark Whelpling'),
(10696, 1791, 'Azure Whelpling'),
(10697, 1791, 'Crimson Whelpling'),
(10698, 1791, 'Emerald Whelpling'),
(10699, 1791, 'Summon Bronze Whelpling'),
(10700, 1791, 'Summon Faeling'),
(10701, 1791, 'Summon Dart Frog'),
(10702, 1791, 'Summon Island Frog'),
(10703, 1791, 'Wood Frog'),
(10704, 1791, 'Tree Frog'),
(10705, 1791, 'Summon Eagle Owl'),
(10706, 1791, 'Hawk Owl'),
(10707, 1791, 'Great Horned Owl'),
(10708, 1791, 'Summon Snowy Owl'),
(10709, 1791, 'Brown Prairie Dog'),
(10710, 1791, 'Summon Cottontail Rabbit'),
(10711, 1791, 'Snowshoe Rabbit'),
(10712, 1791, 'Summon Spotted Rabbit'),
(10713, 1791, 'Albino Snake'),
(10714, 1791, 'Black Kingsnake'),
(10715, 1791, 'Summon Blue Racer'),
(10716, 1791, 'Brown Snake'),
(10717, 1791, 'Crimson Snake'),
(10718, 1791, 'Green Water Snake'),
(10719, 1791, 'Ribbon Snake'),
(10720, 1791, 'Scarlet Snake'),
(10721, 1791, 'Summon Elven Wisp'),
(12243, 1791, 'Mechanical Chicken'),
(13548, 1791, 'Westfall Chicken'),
(15048, 1791, 'Pet Bombling'),
(15049, 1791, 'Lil\' Smoky'),
(15067, 1791, 'Sprite Darter Hatchling'),
(15648, 1791, 'Corrupted Kitten'),
(15999, 1791, 'Worg Pup'),
(16450, 1791, 'Smolderweb Hatchling'),
(17468, 1791, 'Pet Fish'),
(17469, 1791, 'Pet Stone'),
(17567, 1791, 'Summon Blood Parrot'),
(17707, 1791, 'Panda Cub'),
(17708, 1791, 'Mini Diablo'),
(17709, 1791, 'Zergling'),
(19363, 1791, 'Summon Mechanical Yeti'),
(19772, 1791, 'Lifelike Toad'),
(23428, 1791, 'Albino Snapjaw'),
(23429, 1791, 'Loggerhead Snapjaw'),
(23430, 1791, 'Olive Snapjaw'),
(23431, 1791, 'Leatherback Snapjaw'),
(23432, 1791, 'Hawksbill Snapjaw'),
(23530, 1791, 'Tiny Red Dragon'),
(23531, 1791, 'Tiny Green Dragon'),
(23811, 1791, 'Jubling'),
(24696, 1791, 'Murky'),
(24985, 1791, 'Summon Baby Murloc (Blue)'),
(24986, 1791, 'Summon Baby Murloc (Green)'),
(24987, 1791, 'Summon Baby Murloc (Orange)'),
(24988, 1791, 'Lurky'),
(24989, 1791, 'Summon Baby Murloc (Pink)'),
(24990, 1791, 'Summon Baby Murloc (Purple)'),
(25018, 1791, 'Murki'),
(25162, 1791, 'Disgusting Oozeling'),
(25849, 1791, 'Baby Shark'),
(26010, 1791, 'Tranquil Mechanical Yeti'),
(26045, 1791, 'Tiny Snowman'),
(26067, 1791, 'Summon Mechanical Greench'),
(26391, 1791, 'Tentacle Call'),
(26529, 1791, 'Winter Reindeer'),
(26533, 1791, 'Father Winter\'s Helper'),
(26541, 1791, 'Winter\'s Little Helper'),
(27241, 1791, 'Gurky'),
(27570, 1791, 'Peddlefeet'),
(28487, 1791, 'Terky'),
(28505, 1791, 'Poley'),
(28738, 1791, 'Speedy'),
(28739, 1791, 'Mr. Wiggles'),
(28740, 1791, 'Whiskers the Rat'),
(28871, 1791, 'Spirit of Summer'),
(30152, 1791, 'White Tiger Cub'),
(30156, 1791, 'Hippogryph Hatchling'),
(32298, 1791, 'Netherwhelp'),
(33050, 1791, 'Magical Crawdad'),
(33057, 1791, 'Summon Mighty Mr. Pinchy'),
(35156, 1791, 'Mana Wyrmling'),
(35157, 1791, 'Summon Spotted Rabbit'),
(35239, 1791, 'Brown Rabbit'),
(35907, 1791, 'Blue Moth'),
(35909, 1791, 'Red Moth'),
(35910, 1791, 'Yellow Moth'),
(35911, 1791, 'White Moth'),
(36027, 1791, 'Golden Dragonhawk Hatchling'),
(36028, 1791, 'Red Dragonhawk Hatchling'),
(36029, 1791, 'Silver Dragonhawk Hatchling'),
(36031, 1791, 'Blue Dragonhawk Hatchling'),
(36034, 1791, 'Firefly'),
(39181, 1791, 'Miniwing'),
(39709, 1791, 'Wolpertinger'),
(40319, 1791, 'Lucky'),
(40405, 1791, 'Lucky'),
(40549, 1791, 'Bananas'),
(40613, 1791, 'Willy'),
(40614, 1791, 'Egbert'),
(40634, 1791, 'Peanut'),
(40990, 1791, 'Stinker'),
(42609, 1791, 'Sinister Squashling'),
(43697, 1791, 'Toothy'),
(43698, 1791, 'Muckbreath'),
(43918, 1791, 'Mojo'),
(44369, 1791, 'Pint-Sized Pink Pachyderm'),
(45082, 1791, 'Tiny Sporebat'),
(45125, 1791, 'Rocket Chicken'),
(45127, 1791, 'Dragon Kite'),
(45174, 1791, 'Golden Pig'),
(45175, 1791, 'Silver Pig'),
(45890, 1791, 'Scorchling'),
(46425, 1791, 'Snarly'),
(46426, 1791, 'Chuck'),
(46599, 1791, 'Phoenix Hatchling'),
(48406, 1791, 'Spirit of Competition'),
(48408, 1791, 'Essence of Competition'),
(49964, 1791, 'Ethereal Soul-Trader'),
(51716, 1791, 'Nether Ray Fry'),
(51851, 1791, 'Vampiric Batling'),
(52615, 1791, 'Frosty'),
(53082, 1791, 'Mini Tyrael'),
(53316, 1791, 'Ghostly Skull'),
(53768, 1791, 'Haunted'),
(54187, 1791, 'Clockwork Rocket Bot'),
(55068, 1791, 'Mr. Chilly'),
(59250, 1791, 'Giant Sewer Rat'),
(61348, 1791, 'Tickbird Hatchling'),
(61349, 1791, 'White Tickbird Hatchling'),
(61350, 1791, 'Proto-Drake Whelp'),
(61351, 1791, 'Cobra Hatchling'),
(61357, 1791, 'Pengu'),
(61472, 1791, 'Kirin Tor Familiar'),
(61725, 1791, 'Spring Rabbit'),
(61773, 1791, 'Plump Turkey'),
(61855, 1791, 'Baby Blizzard Bear'),
(61991, 1791, 'Little Fawn'),
(62491, 1791, 'Teldrassil Sproutling'),
(62508, 1791, 'Dun Morogh Cub'),
(62510, 1791, 'Tirisfal Batling'),
(62513, 1791, 'Durotar Scorpion'),
(62514, 1791, 'Alarming Clockbot'),
(62516, 1791, 'Elwynn Lamb'),
(62542, 1791, 'Mulgore Hatchling'),
(62561, 1791, 'Strand Crawler'),
(62562, 1791, 'Ammen Vale Lashling'),
(62564, 1791, 'Enchanted Broom'),
(62609, 1101, 'Argent Squire'),
(62674, 1791, 'Mechanopeep'),
(62746, 690, 'Argent Gruntling'),
(63318, 1791, 'Murkimus the Gladiator'),
(63712, 1791, 'Sen\'jin Fetish'),
(64351, 1791, 'XS-001 Constructor Bot'),
(65358, 1791, 'Calico Cat'),
(65381, 1791, 'Curious Oracle Hatchling'),
(65382, 1791, 'Curious Wolvar Pup'),
(65682, 1791, 'Warbot'),
(66030, 1791, 'Grunty'),
(66096, 1791, 'Shimmering Wyrmling'),
(66175, 1791, 'Macabre Marionette'),
(66520, 1791, 'Jade Tiger'),
(67413, 1791, 'Darting Hatchling'),
(67414, 1791, 'Deviate Hatchling'),
(67415, 1791, 'Gundrak Hatchling'),
(67416, 1791, 'Leaping Hatchling'),
(67417, 1791, 'Obsidian Hatchling'),
(67418, 1791, 'Ravasaur Hatchling'),
(67419, 1791, 'Razormaw Hatchling'),
(67420, 1791, 'Razzashi Hatchling'),
(67527, 1791, 'Onyx Panther'),
(68767, 1791, 'Tuskarr Kite'),
(68810, 1791, 'Spectral Tiger Cub'),
(69002, 1791, 'Onyxian Whelpling'),
(69452, 1791, 'Core Hound Pup'),
(69535, 1791, 'Gryphon Hatchling'),
(69536, 1791, 'Wind Rider Cub'),
(69539, 1791, 'Zipao Tiger'),
(69541, 1791, 'Pandaren Monk'),
(69677, 1791, 'Lil\' K.T.'),
(70613, 1791, 'Perky Pug'),
(71840, 1791, 'Toxic Wasteling'),
(74932, 1791, 'Frigid Frostling'),
(75134, 1791, 'Blue Clockwork Rocket Bot'),
(75613, 1791, 'Celestial Dragon'),
(75906, 1791, 'Lil\' XT'),
(75936, 1791, 'Murkimus the Gladiator'),
(78381, 1791, 'Mini Thor');
