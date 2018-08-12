# DONE list

## Countdown

* Add an "env_timer" that act as a countdown.

## Items changes

* Weaponbox respawns after being used.

## Message/text window

Add an "env_paper" entity that display a text file in a MOTD style window.

## Weapons removal

Remove the following Half-Life weapons:

* Gauss gun.
* Egon.
* Tripmine.
* Satchel.
* Hornet gun.
* Snarks.

## Weapons changes

* Crowbar: -50% attack speed.
* Glock: -50% attack speed, no secondary fire, 12 bullets in the mag instead of 17.
* MP5: secondary fire is a zoom (like HL's crossbow) with perfect accuracy. Don't produce muzzleflash when zooming. Use it's own ammo.
* Shotgun: `ammo_buckshot` gives 7 buckshot shells instead of 12. -50% attack speed, no secondary fire.

## Player changes

* Reduce normal speed by 20%.
* Reduce speed by 70% if equals or less than 30 HP.

## Zombie changes

* Change blood and gibs to red.
* Stun player for 2 seconds.

## Items changes

* Items on the floor should be picked up by using them not by walking on them.

## HUD changes

Replace the health digits by those sprites (remove the color as well):

* `sprites/ECGFine.spr` between 71 and 100 HP.
* `sprites/ECGCaution.spr` between 31 and 70 HP.
* `sprites/ECGDanger.spr` between 1 and 30 HP.

## New weapons

* Grenade launcher (grenades explode on contact).
* Red herb: heals 25%.
* Green herb: heals 50%.
* Spray: heals 100%.
* Flamethrower.

## Entity info

Like Sven Co-op, add entity info for weapons/items/NPCs to show their name and health. Color them in red for enemies, green for friends and yellow for the rest.
