-- Date Created: July, 2022
-- Updated: January, 2023

-- API Services
local ReplicatedStorage = game:GetService("ReplicatedStorage")

-- Required Dependencies
local UsefulFunctions = require(ReplicatedStorage.ModuleScripts.UsefulFunctions)
local Particles = require(ReplicatedStorage.ModuleScripts.Particles)

-- PRIVATE LOCAL VARIABLES
local RESPAWN_TIME = 30

local Destructibles = {}
local DestructiblesTable = {}

Destructibles.__index = Destructibles



function Destructibles:GetDestructibleFromModel(model)
    return DestructiblesTable[model]
end

function Destructibles.new(instance)
    local newObject = {}
    setmetatable(newObject, Destructibles)


    -- Variables
    newObject.Instance = instance
    newObject.MaxHP = 100 --default value


    -- Create HitBox
    newObject.HitBox = UsefulFunctions:CreateHitBox(instance)
    newObject.HitBox.CanQuery = false
    newObject.HitBox.CanCollide = false

    DestructiblesTable[newObject.Instance] = newObject

    return newObject
end

function Destructibles:Reveal(bool)
	for _, part in pairs(self.Instance:GetDescendants()) do
	    if part:IsA("BasePart") and part.Name ~= "HitBox" then
		part.CanCollide = bool
		part.Transparency = (bool and 0) or 1
	    end
	end	
end

function Destructibles:Init()
    -- Sets attribute health
    self.Instance:SetAttribute("Health", self.MaxHP)

    -- Determine if this is not a resource that drops particular assets. (All classes should be able to destroy it otherwise)
    if self.Name ~= "Tree" and self.Name ~= "CrystalRock" then
        self.Instance:SetAttribute("WorkerInteractable", true) -- Makes it so that workers can repair and perform misc. actions against it.
    end
    
    local debounce = false

	self.Connection = self.Instance:GetAttributeChangedSignal("Health"):Connect(function()
            if self.Sounds then -- Randomizes sound
                self.randEffect = math.random(#self.Sounds)
                self.HitSound.SoundId = "rbxassetid://" .. self.Sounds[self.randEffect]
            end
            
	-- Plays tree hit animation
            if self.HitAnimTrack then
                self.HitAnimTrack:Play()
            end
            
            -- gets the current HP of the destructible
            local currentHp = self.Instance:GetAttribute("Health")

            if currentHp > 0 and currentHp ~= self.MaxHP then
                self:Hit()
                self.HitSound:Play()
            end

            -- When the destructible's hp is 0 then the destructible "disappears", drops a resource, then reappears after a certain amount of time
            if currentHp <= 0 and debounce == false then
                debounce = true
		self:Reveal(true)

                if self.DropResource then
                    self:DropResource()
                end

                self.HitSound:Play()
                

                task.wait(RESPAWN_TIME)

                self.Instance:SetAttribute("Health", self.MaxHP)
		self:Reveal(false)
                
                debounce = false
            end
        
    end)
end

return Destructibles
