// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"


void UMenu::MenuSetup(int32 numberOfPublicConnections, FString typeOfMatch, FString lobbyPath)
{
    pathToLobby = FString::Printf(TEXT("%s?listen"), *lobbyPath);
    numPublicConnections = numberOfPublicConnections;
    matchType = typeOfMatch;
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    UWorld* world = GetWorld();
    if (world)
    {
        APlayerController* playerController = world->GetFirstPlayerController();
        if (playerController)
        {
            FInputModeUIOnly inputModeData;
            inputModeData.SetWidgetToFocus(TakeWidget());
            inputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            playerController->SetInputMode(inputModeData);
            playerController->SetShowMouseCursor(true);
        }
    }

    UGameInstance* gameInstance = GetGameInstance();
    if (gameInstance)
    {
        multiplayerSessionsSubsystem = gameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    }

    if (multiplayerSessionsSubsystem)
    {
        multiplayerSessionsSubsystem->multiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
        multiplayerSessionsSubsystem->multiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
        multiplayerSessionsSubsystem->multiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
        multiplayerSessionsSubsystem->multiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
        multiplayerSessionsSubsystem->multiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
    }
}

bool UMenu::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (HostButton)
    {
        HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
    }

    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
    }

    return true;
}

void UMenu::OnLevelRemovedFromWorld(ULevel *InLevel, UWorld *InWorld)
{
    MenuTearDown();
    Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::HostButtonClicked()
{
    // Creating Session
    HostButton->SetIsEnabled(false);
    if (multiplayerSessionsSubsystem)
    {
        multiplayerSessionsSubsystem->CreateSession(numPublicConnections, matchType);
    }
}

void UMenu::JoinButtonClicked()
{
    // Finding the correct session
    JoinButton->SetIsEnabled(false); 
    if (multiplayerSessionsSubsystem)
    {
        multiplayerSessionsSubsystem->FindSessions(10000);
    }
}

void UMenu::MenuTearDown()
{
    RemoveFromParent();
    UWorld* world = GetWorld();
    if (world)
    {
        APlayerController* playerController = world->GetFirstPlayerController();
        if (playerController)
        {
            FInputModeGameOnly inputMode;
            playerController->SetInputMode(inputMode);
            playerController->bShowMouseCursor = false; 
        }
    }
}

void UMenu::OnCreateSession(bool bWasSuccsessful)
{
    // Call ServerTravel from UWorld* here after the session has been successfully created
    if (bWasSuccsessful)
    {
        UWorld *world = GetWorld();
        if (world)
        {
            // C:/Epic Games/Unreal Projects/Multiplayer_Plugin/Content/ThirdPerson/Maps/Lobby.umap
            world->ServerTravel(pathToLobby);
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Red,
                FString("Failed to create session!"));
        }
        HostButton->SetIsEnabled(true);
    }
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& sessionResults, bool bWasSuccessful)
{
    // currently only looping through the sessionResults TArray to find the correct session
    // to be refined later?

    if (multiplayerSessionsSubsystem == nullptr) return;

    for (auto result : sessionResults)
    {
        FString settingsValue;
        result.Session.SessionSettings.Get(FName("MatchType"), settingsValue);
        if (settingsValue == matchType)
        {
            multiplayerSessionsSubsystem->JoinSession(result);
            return;
        }
    }
    if (!bWasSuccessful || sessionResults.IsEmpty())
    {
        JoinButton->SetIsEnabled(true);
    }
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type result)
{
    // Call Client travel here
    IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
    if (onlineSubsystem)
    {
        IOnlineSessionPtr sessionInterface = onlineSubsystem->GetSessionInterface();
        if (sessionInterface.IsValid())
        {
            FString address;
            sessionInterface->GetResolvedConnectString(NAME_GameSession, address);

            APlayerController *playerController = GetGameInstance()->GetFirstLocalPlayerController();
            if (playerController)
            {
                playerController->ClientTravel(address, ETravelType::TRAVEL_Absolute);
            }
        }
    }
    
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
    //

}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
    //
    
}