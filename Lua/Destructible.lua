local ReplicatedStorage = game:GetService("ReplicatedStorage")

- References other module scripts
local UsefulFunctions = require(ReplicatedStorage.ModuleScripts.UsefulFunctions)
local Particles = require(ReplicatedStorage.ModuleScripts.Particles)

local Destructibles = {}
local DestructiblesTable = {}

Destructibles.__index = Destructibles
local RESPAWN_TIME = 30

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

function Destructibles:GetDestructibleFromModel(model)
    for _, object in pairs(DestructiblesTable) do
        if object.Instance == model then
            return object
        end
    end
end


function Destructibles:Init()
    -- Sets attribute health
    self.Instance:SetAttribute("Health", self.MaxHP)

    -- Determine if this is not a resource that drops. (All classes should be able to destroy it otherwise)
    if self.Name ~= "Tree" and self.Name ~= "CrystalRock" then
        self.Instance:SetAttribute("WorkerInteractable", true)
    end
    
    local debounce = false

	self.Connection = self.Instance:GetAttributeChangedSignal("Health"):Connect(function()
            if self.Sounds then -- Randomizes sound
                self.randEffect = math.random(#self.Sounds)
                self.HitSound.SoundId = "rbxassetid://" .. self.Sounds[self.randEffect]
            end
            
            if self.HitAnimTrack then
                self.HitAnimTrack:Play()
            end
            
            -- gets the current HP of the destructible
            local currentHp = self.Instance:GetAttribute("Health")

            if currentHp > 0 and currentHp ~= self.MaxHP then
                self:Hit()
                self.HitSound:Play()
            end

            -- When the destructible's hp is 0 then the destructible "disapears", drops a resource, then reappears after a certain amount of time
            if currentHp <= 0 and debounce == false then
                debounce = true
                for _, part in pairs(self.Instance:GetDescendants()) do
                    if part:IsA("BasePart") then
                        part.CanCollide = false
                        part.Transparency = 1
                    end
                end

                if self.DropResource then
                    self:DropResource()
                end

                self.HitSound:Play()
                

                task.wait(RESPAWN_TIME)

                self.Instance:SetAttribute("Health", self.MaxHP)
                for _, part in pairs(self.Instance:GetDescendants()) do
                    if part:IsA("BasePart") then
                        if part.Name ~= 'HitBox' then
                            part.CanCollide = true
                            part.Transparency = 0
                        end
                    end
                end
                
                debounce = false
            end
        
    end)
end

return Destructibles
