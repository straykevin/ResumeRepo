-- DATE: July, 2022

-- ROBLOX APIs
local RunService = game:GetService("RunService")
local ServerScriptService = game:GetService("ServerScriptService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Players = game:GetService("Players")

-- Module Scripts
local Classes = require(ReplicatedStorage.ModuleScripts.Classes)
local Assets = ReplicatedStorage.Assets
local Remotes = ReplicatedStorage:WaitForChild("Remotes")
local Particles = require(ReplicatedStorage.ModuleScripts.Particles)
local Sounds = require(ReplicatedStorage.ModuleScripts.Sounds)


local PlayerManager

if RunService:IsServer() then
    PlayerManager = require(ServerScriptService.ModuleScripts.Utilities.PlayerManager)
end


local Worker = {}
Worker.__index = Worker
setmetatable(Worker, Classes)


function Worker.new(character, level)
    local newWorker = Classes.new(character, level)
    setmetatable(newWorker, Worker)

    newWorker.ClassName = "Worker"

    newWorker.ToolDir = Assets.Tools.Axe:Clone()
    newWorker.HatDir = Assets.Hats.WorkerHat:Clone()
    newWorker.HatOffset = Vector3.new(0, -1.531, 0)

    newWorker.BasicAnimDir = newWorker:CreateAnim("BasicAnim", 10048190135)
    newWorker.SecondAnimDir = newWorker:CreateAnim("SecondAnim", 10017349127)
    newWorker.SecondAbilityCost = 0

    newWorker.InteractParams = OverlapParams.new()
    newWorker.InteractParams.FilterType = Enum.RaycastFilterType.Blacklist

    
    newWorker.RepairRate = 30 --HP/sec
    newWorker.RepairCost = 10 --Essence/sec -- Currently should repair 3HP/essence. Gate should be at 300hp with 1 chargepoint
    newWorker:Init()
    newWorker:Reset()
    return newWorker
end


function Worker:OnHitExtra(hit)
    local resourceNode = hit.Parent
  
    if resourceNode:IsA("Model") and not resourceNode:GetAttribute("WorkerInteractable") then
        local CurrentHP = resourceNode:GetAttribute("Health")
        if CurrentHP and CurrentHP > 0 then
            print(self.Character, "has hit a resource:",resourceNode)

            local plr = game.Players:GetPlayerFromCharacter(resourceNode)

            if plr then
                Remotes.PlayerHitEvent:FireClient(plr, resourceNode, self.Damage * self.DamageMultiplier)
            end

            Remotes.PlayerHitEvent:FireClient(Players:GetPlayerFromCharacter(self.Character), resourceNode, self.Damage * self.DamageMultiplier)
            resourceNode:SetAttribute("Health", CurrentHP - (self.Damage * self.DamageMultiplier))

            local Destructibles = require(ServerScriptService.ModuleScripts.Game.Destructibles)

            local hitInstance = Destructibles:GetDestructibleFromModel(resourceNode)
            
            if hitInstance then
                if hitInstance.Name == "Tree" and hitInstance.Instance:GetAttribute("Health") <= 0 then
                    PlayerManager:IncrementWood(game.Players:GetPlayerFromCharacter(self.Character), 1)
                elseif hitInstance.Name == "CrystalRock" and hitInstance.Instance:GetAttribute("Health") <= 0 then
                    PlayerManager:IncrementCrystals(game.Players:GetPlayerFromCharacter(self.Character), 1)
                end
            end

            PlayerManager:IncrementEssence(Players:GetPlayerFromCharacter(self.Character), 5)
        end
    end

end


function Worker:CastPrimary()
    self.PrimaryParticle1 = Particles.new("LightRayCharge", self.Tool.BladeHolder)
end


function Worker:CastSecondary(bool)

    if self.WorkerWorking then
        self.WorkerWorking:Disconnect()
    end

    local hrp = self.Character.HumanoidRootPart

    if bool then -- START WORKER INTERACTION
        -- Interacts with closest interactable model
        self.InteractParams.FilterDescendantsInstances = {self.Character}
        local partsAroundPlayer = workspace:GetPartBoundsInRadius(hrp.Position, 15, self.InteractParams)

        table.sort(partsAroundPlayer, function(a, b) -- sort by closest parts
            return (a.Position - hrp.Position).Magnitude < (b.Position - hrp.Position).Magnitude
        end)

        local closestModel = nil
        for _, v in pairs(partsAroundPlayer) do -- It's in distance order, so get the first Model with the attribute
            if v.Parent:GetAttribute("WorkerInteractable") then
                closestModel = v.Parent
                break
            elseif v.Parent.Parent:GetAttribute("WorkerInteractable") then
                closestModel = v.Parent.Parent
                break
            end
        end


        if not closestModel then 
            ReplicatedStorage.Remotes.GameTipEvent:FireClient(Players:GetPlayerFromCharacter(self.Character), "Use your second ability next to a hat machine or gate to repair/level up", 3)
            return false 
        end

        local SplitString = string.split(string.sub(string.gsub(closestModel.Name, "(%u)", " %1"), 2, -1), " ")
        if SplitString[1] == "Machine" then
            self:Upgrade(closestModel)
        else
            self:Repair(closestModel)
        end

    else -- STOP WORKER INTERACTION
        --releasing key will disconnect self.WorkerWorking

        hrp.Anchored = false

        if self.RepairSound then
            self.RepairSound:Stop()
            self.RepairSound:Destroy()
        end
    end

end


function Worker:Repair(repairModel)
    local player = Players:GetPlayerFromCharacter(self.Character)
    local hrp = self.Character.HumanoidRootPart

    if RunService:IsServer() then

        
        local object 
        
        object = require(ServerScriptService.ModuleScripts.Game.Destructibles.Gate):GetGate(repairModel) -- NEED TO CHANGE THIS TO ACCOMODATE FUTURE REPAIR OBJECTS
        if not object then
            object = require(ServerScriptService.ModuleScripts.Game.Destructibles.Tunnel):GetTunnel(repairModel)
        end

        if object then
            local objectTeam = object.Team

            local playerTeam = player.Team

            if objectTeam == playerTeam then
                hrp.Anchored = true -- Anchor character in place while repairing
                local playerInfo = PlayerManager:FindPlayer(player)


                self.RepairSound = Sounds:CreateSound("RepairSound", hrp, true)
                self.RepairSound:Play()

                self.WorkerWorking = RunService.Heartbeat:Connect(function(dt)

                    if playerInfo.Essence >= dt * self.RepairCost and repairModel:GetAttribute("Health") < object.MaxHP then
                        PlayerManager:IncrementEssence(player, -(dt * self.RepairCost))
                        repairModel:SetAttribute("Health", repairModel:GetAttribute("Health") + (dt * self.RepairRate))
                    else -- not enough essence to keep repairing or the object is now fully finished.
                        if playerInfo.Essence <= dt * self.RepairCost then
                            Remotes.MessageClientEvent:FireClient(player, "You need more essence to repair this.")
                        end

                        if self.WorkerWorking then

                            if self.RepairSound then
                                print("Stopping repair sound!")
                                self.RepairSound:Stop()
                                self.RepairSound:Destroy()
                            else
                                warn("Sound was not found!")
                            end
                            self.WorkerWorking:Disconnect()
                            self:EndSecondAnim({})
                        end
                    end
                end)
                
            else
                warn("Worker", player, "is on", playerTeam, "trying to fix", objectTeam, repairModel.Name)
                return false
            end
        end
    end
    
    return true
end

function Worker:Upgrade(machineModel: Model)
    local player = Players:GetPlayerFromCharacter(self.Character)

    if RunService:IsServer() then
        local machine = require(ServerScriptService.ModuleScripts.Game.HatMachine):GetHatMachine(machineModel)
        machine:LevelUP(player)
    end
    return true
end

return Worker
